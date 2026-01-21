#include "config.h"
#include <void/void.h>
#include <void/util/error.h>
#include "config_drawable.h"

#if defined(R2_PLATFORM_WINDOWS)
#include <Windows.h>
#include <ShlObj.h>
#endif


void_begin_

inline static std::string format_write_time(const std::chrono::time_point<std::chrono::file_clock>& ftime) {
    auto stime = std::chrono::clock_cast<std::chrono::system_clock>(ftime);

    auto day_point = std::chrono::floor<std::chrono::days>(stime);
    std::chrono::year_month_day ymd{ day_point };
    std::chrono::hh_mm_ss time_of_day{ stime - day_point };

    // extract types
    const unsigned day = static_cast<unsigned>(ymd.day());
    const unsigned hour = static_cast<unsigned>(time_of_day.hours().count());
    const unsigned minute = static_cast<unsigned>(time_of_day.minutes().count());

    return std::format(
        "{:%b} {:02d}, {}:{:02d}",
        stime,
        day,
        hour, 
        minute
    );
}

inline static std::string get_write_time(const std::filesystem::path& p) {
    auto ftime = std::filesystem::last_write_time(p);

    return format_write_time(ftime);
}

_config::_config(void_* instance)
    : config_manager(instance, 
        std::wstring(kConfigExtension.begin(), kConfigExtension.end()), 
        "config"
      ),
      main_path_()
{
}

_config::~_config() = default;

void _config::init()
{
#if defined(R2_PLATFORM_WINDOWS)
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    bool did_init = SUCCEEDED(hr);
    if (hr == RPC_E_CHANGED_MODE) {
        did_init = false;
        hr = S_OK;
    }
    if (FAILED(hr)) {
        throw error(error_code::config_init,
            1,
            static_cast<std::int32_t>(hr)
        );
    }

    PWSTR path = nullptr;

    hr = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &path);
    if (FAILED(hr)) {
        throw error(error_code::config_init,
            1,
            static_cast<std::int32_t>(hr)
        );
    }

    if (did_init) {
        CoUninitialize();
    }

    main_path_.clear();
    main_path_.append(path);
#endif

    const auto& config_name = instance()->options().get<options::option_ConfigName>();
    std::string base_name;
    base_name.reserve(config_name.length());

    for (std::size_t i = 0u; i < config_name.length(); i++)
        base_name += config_name[i];

    main_path_ /= base_name;

    do_init(main_path_ / "cfg");

    if (instance()->options().get<options::option_LoadLastConfig>()) {
        load_last_config();
    }
}

void _config::destroy()
{
    configs_.clear();
    modules_.clear();

    main_path_.clear();
}

std::size_t _config::add_module(std::unique_ptr<config_module>&& module)
{
    // make sure not to add the same name twice
    const xstr& name = module->get_name();
    const xstr original_name = name;
    int num = 0;
    for (;;) {
        bool found = false;
        for (auto& m : modules_) {
            if (m) {
                if (m->get_name() == name) {
                    found = true;
                    break;
                }
            }
        }

        if (!found)
            break;

        xstr new_name;
        new_name.append(std::to_string(num).c_str());
        module->set_name(new_name + original_name);
        num++;
    }

    if (!freed_modules_.empty()) {
        const std::size_t id = freed_modules_.back();
        assert(id < modules_.size());
        assert(!modules_[id]);
        modules_[id] = std::move(module);

        freed_modules_.pop_back();
        return id;
    }
    else {
        modules_.push_back(std::move(module));

        return modules_.size() - 1u;
    }
}

void _config::remove_module(std::size_t module)
{
    assert(module < modules_.size());

    modules_[module].reset();
    freed_modules_.push_back(module);
}

bool _config::create_new()
{
    std::wstring path;
    if (!do_create_new(path))
        return false;

    // add
    configs_.push_back(
        std::make_unique<config_drawable>(
            instance(), 
            instance(), /* input owner */
            this,
            path,
            get_write_time(get_file_path(path))
        )
    );

    // notification
    auto& style = instance()->style();
    instance()->notifications().create_note()
        << style.text_accent() << xstr("Config created ")
        << style.accent() << path
        << style.text() << xstr(".");

    return true;
}

/*
Format:
name (null terminated)
size (4 bytes)
data (must match size)
*/

bool _config::load(const std::wstring& name, bool first)
{
    std::vector<std::uint8_t> buffer;
    if (!do_load(name, buffer))
        return false;
    
    // reset
    for (auto& mod : modules_) {
        if (mod)
            mod->reset();
    }

    auto data = buffer.begin();

    if (buffer.empty()) {
        goto done;
    }

    // parse data
    while (data < buffer.end()) {
        const char* module_name = reinterpret_cast<const char*>(&(*data));

        auto data_backup = data;
        bool found = false;
        for (auto& mod : modules_) {
            if (!mod)
                continue;

            if (reinterpret_cast<const std::uint8_t*>(
                    module_name + mod->get_name().length() + 1) >
                buffer.data() + buffer.size()) {
                continue;
            }

            if (mod->get_name() != module_name) { // == operator will never go past length + 1, so this is okay
                continue;
            }

            data += mod->get_name().length();
            data += 1; // '\0'

            std::uint32_t data_length;

            if (data + sizeof(data_length) >= buffer.end()) {
                break;
            }

            std::memcpy(&data_length, &(*data), sizeof(data_length));

            if (data_length != mod->get_size()) {
                break;
            }

            data += sizeof(data_length);

            if (data + data_length > buffer.end()) {
                break;
            }

            mod->load(&(*data));

            data += data_length;

            found = true;
            break;
        }

        if (!found) {
            data = data_backup;

            auto str_len = strnlen_s(module_name,
                static_cast<std::size_t>(buffer.end() - data));

            if (str_len == static_cast<std::size_t>(buffer.end() - data) ||
                str_len > xstr::kMaxSize) { // invalid config data
                show_error(xstr("Failed to parse config"), &name);
                return false;
            }

            data += str_len;
            data += 1; // '\0'

            std::uint32_t data_length;
            if (data + sizeof(data_length) >= buffer.end())
                break;

            std::memcpy(&data_length, &(*data), sizeof(data_length));

            data += sizeof(std::uint32_t);

            data += data_length;
        }
    }

done:
    if (!first)
        save_last_file(name);

    if (!first) {
        // notification
        auto& style = instance()->style();
        instance()->notifications().create_note()
            << style.text_accent() << type_name_pascal_ << xstr(" loaded ")
            << style.grey() << name
            << style.text() << xstr(".");
    }

    instance()->callbacks().invoke<callbacks::callback_OnLoadConfig>();

    return true;
}

bool _config::save(const std::wstring& name, config_drawable* drawable)
{
    std::vector<std::uint8_t> data;
    data.reserve(modules_.size() * 10); // default estimated size

    for (const auto& mod : modules_) {
        if (!mod)
            continue;

        // save name
        for (std::uint8_t i = 0; i < mod->get_name().length(); i++)
            data.push_back(mod->get_name()[i]);

        data.push_back('\0');

        // save size
        std::uint32_t data_length = mod->get_size();

        auto old_size = data.size();
        data.resize(old_size + sizeof(data_length));

        std::memcpy(data.data() + old_size, &data_length, sizeof(data_length));

        // save data
        mod->save(data);
    }

    if (data.empty()) {
        goto done;
    }

    // save file
    if (!do_save(name, data.data(), data.size()))
        return false;

    // read updated time
    drawable->set_modified_time(
        get_write_time(get_file_path(name))
    );

done:
    save_last_file(name);

    // notification
    auto& style = instance()->style();
    instance()->notifications().create_note()
        << style.text_accent() << type_name_pascal_ << xstr(" saved ")
        << style.grey() << name
        << style.text() << xstr(".");

    instance()->callbacks().invoke<callbacks::callback_OnSaveConfig>();

    return true;
}

bool _config::remove(const std::wstring& name, config_drawable* drawable)
{
    if (!do_remove(name))
        return false;

    drawable->queue_delete();

    return true;
}

bool _config::rename(const std::wstring& name, const std::wstring& new_name)
{
    return do_rename(name, new_name);
}

void _config::load_last_config()
{
    const std::wstring* v;
    if (load_last_file(v))
        load(*v, true);
}

bool _config::refresh_configs(std::int32_t& selected_config)
{
    std::wstring cfg_name;
    if (selected_config >= 0 &&
        selected_config < static_cast<std::int32_t>(configs_.size()))
        cfg_name = configs_[selected_config]->get_name();

    std::error_code ec;
    auto iterator = std::filesystem::directory_iterator(get_folder_path(), ec);
    if (ec) {
        show_error(xstr("Failed to list configs"), nullptr, ec);
        return false;
    }

    for (auto& cfg : configs_)
        cfg->seen_in_refresh() = false;

    std::int32_t selected_index = 0;
    for (auto file : iterator) {
        if (!file.is_regular_file())
            continue;

        if (file.path().extension().string() != kConfigExtension)
            continue;

        auto str = file.path().filename().wstring();
        for (std::size_t i = 0u; i < kConfigExtension.length(); i++)
            str.pop_back();

        // update selected_index
        if (!cfg_name.empty() &&
            cfg_name == str)
            selected_index = static_cast<std::int32_t>(configs_.size()) - 1;

        // check if exists
        bool found = false;
        for (auto& cfg : configs_) {
            if (cfg->get_name() == str) {
                cfg->seen_in_refresh() = true;
                found = true;
                break;
            }
        }

        // add
        if (!found) {
            configs_.push_back(
                std::make_unique<config_drawable>(
                    instance(), 
                    instance(), /* input owner */
                    this,
                    str,
                    format_write_time(file.last_write_time())
                )
            );
        }
    }

    // check not found cfgs
    for (auto& cfg : configs_)
        if (!cfg->seen_in_refresh())
            cfg->queue_delete();

    update(selected_index);

    return true;
}

void _config::update(std::int32_t& selected_index)
{
    std::int32_t i = 0;
    for (auto it = configs_.begin(); it != configs_.end();) {
        if (it->get()->can_be_deleted()) {
            if (i == selected_index)
                selected_index = -1;
            else if (i < selected_index)
                selected_index--;
            it = configs_.erase(it);
            i--;
        }
        else
            it++;
        i++;
    }
}

void_end_