#include "config_manager.h"
#include <void/util/error.h>
#include <void/void.h>
#include <format>
#include <fstream>


template<>
struct std::formatter<vo::xstr> : std::formatter<std::string_view> {
    auto format(const vo::xstr& value, auto& ctx) const {
        char buffer[value.kMaxSize];
        value.get(buffer);

        std::string_view sv{ buffer, value.length() };
        return std::formatter<std::string_view>::format(sv, ctx);
    }
};

void_begin_

// error 
inline static constexpr xstr kPathMark = "/path/";
template<typename... Args>
void show_error(void_* instance, const std::wstring* path, const std::error_code& err, std::format_string<Args...> f, Args&&... args) {
    auto& style = instance->style();

    auto e = instance->notifications().create_error();

    std::string formatted = std::format(f, std::forward<Args>(args)...);
    if (path != nullptr) {
        constexpr std::string_view search = "/path/";
        std::size_t path_start = formatted.find(search);
        assert(path_start != std::string::npos);
        std::size_t path_end = path_start + search.length();

        std::string left(formatted.c_str(), formatted.c_str() + path_start);

        std::string right(formatted.c_str() + path_end, formatted.c_str() + formatted.length());
        
        e << left;

        e << style.grey() << *path;

        e << right;
    }
    else {
        e << formatted;
    }

    if (err) {
        e << style.text() << xstr(" (");

        using std::errc;
        auto cond = err.default_error_condition();

        if (cond == errc::no_such_file_or_directory) {
            e << xstr("Not Found");
        }
        else if (cond == errc::permission_denied) {
            e << xstr("Access Denied");
        }
        else if (cond == errc::device_or_resource_busy) {
            e << xstr("Sharing Violation or Locked");
        }
        else if (cond == errc::file_exists) {
            e << xstr("File Already Exists");
        }
        else if (cond == errc::no_space_on_device) {
            e << xstr("Disk Full");
        }
        else if (cond == errc::read_only_file_system) {
            e << xstr("Write Protected");
        }
        else if (cond == errc::io_error) {
            e << xstr("I/O Error");
        }
        else {
            e << std::to_string(err.value());
        }

        e << xstr(")");
    }
}


config_manager::config_manager(void_* instance, std::wstring&& extension, const std::string& type_name)
    : vobj(instance),
      extension_(extension)
{
    type_name_lower_.resize(type_name.size());
    std::transform(
        type_name.begin(), type_name.end(),
        type_name_lower_.begin(),
        [](char c) {
            return std::tolower(c);
        }
    );

    type_name_pascal_ = type_name_lower_;
    type_name_pascal_[0] = static_cast<char>(
        std::toupper(static_cast<int>(type_name_pascal_[0]))
    );
}

bool config_manager::do_init(const std::filesystem::path& path)
{
    path_ = path;

    std::error_code ec;
    std::filesystem::create_directories(path, ec);
    if (ec) {
        return false;
    }

    last_file_path_ = path / kLastFileName;

    return true;
}

bool config_manager::load_last_file(const std::wstring*& out)
{
    std::ifstream file(last_file_path_, std::ios::binary | std::ios::ate);
    if (!file)
        return false;

    std::streampos file_size = file.tellg();
    if (file_size == 0ull)
        return false;

    if (file_size % sizeof(wchar_t) != 0)
        return false;

    file.seekg(0ull, std::ios::beg);

    std::vector<wchar_t> buffer;
    buffer.resize(
        static_cast<std::size_t>(file_size) / sizeof(wchar_t)
    );

    file.read(
        reinterpret_cast<char*>(buffer.data()),
        file_size
    );
    if (!file)
        return false;

    file.close();

    if (buffer.back() != L'\0')
        return false;

    last_file_.clear();
    last_file_.assign(
        buffer.data(),
        buffer.data() + buffer.size() - 1u /* remove \0 */
    );

    out = &last_file_;

    return true;
}

bool config_manager::save_last_file(const std::wstring& path)
{
#if defined(_DEBUG)
    const auto pos = path.find_last_of(L'\\');
    assert(pos == path.npos);
#endif

    last_file_ = path;

    std::ofstream file(last_file_path_, std::ios::binary);
    if (!file)
        return false;

    file.write(
        reinterpret_cast<const char*>(path.c_str()), 
        (path.length() + 1) * sizeof(wchar_t)
    );
    if (!file)
        return false;

    file.flush();
    if (!file)
        return false;

    file.close();

    return true;
}

bool config_manager::do_create_new(std::wstring& out)
{
    constexpr int kMaxDefaultConfigs = 40;
    int config_index = 0;

    std::wstring name_begin;
    name_begin.resize(type_name_pascal_.size());

    std::transform(
        type_name_pascal_.begin(), type_name_pascal_.end(),
        name_begin.begin(),
        [](char c) -> wchar_t {
            return static_cast<wchar_t>(c);
        }
    );

    std::wstring name;
    while (true) {
        name = name_begin + std::to_wstring(config_index) + extension_;

        if (!std::filesystem::exists(path_ / name))
            break;

        config_index++;

        if (config_index == kMaxDefaultConfigs) {
            ::vo::show_error(instance(), nullptr, {}, "Max number of default configs reached ({})", config_index);

            return false;
        }
    }

    try {
        std::ofstream file;
        file.exceptions(std::ios::failbit | std::ios::badbit);

        // create file
        file.open(path_ / name);
    }
    catch (const std::ios_base::failure& e) {
        show_error(xstr("Failed to create new config file"), nullptr, e.code());
        return false;
    }

    out = name_begin + std::to_wstring(config_index);

    return true;
}

bool config_manager::do_load(const std::wstring& path, std::vector<std::uint8_t>& out)
{
    try {
        std::ifstream file;
        file.exceptions(std::ios::failbit | std::ios::badbit);

        file.open(get_file_path(path), std::ios::binary | std::ios::ate);

        std::streampos file_size = file.tellg();
        if (file_size <= 0u)
            return true;

        file.seekg(0, std::ios::beg);

        out.clear();
        out.resize(static_cast<std::size_t>(file_size));
        file.read(reinterpret_cast<char*>(out.data()), out.size());

        file.close();
    }
    catch (const std::ios_base::failure& e) {
        ::vo::show_error(instance(), &path, e.code(), "Failed to open {} /path/", type_name_lower_);
        return false;
    }

    return true;
}

bool config_manager::do_save(const std::wstring& path, std::uint8_t* data, std::size_t size)
{
    try {
        std::ofstream file;
        file.exceptions(std::ios::failbit | std::ios::badbit);

        file.open(get_file_path(path), std::ios::binary);

        file.write(reinterpret_cast<char*>(data), size);

        file.flush();
        file.close();
    }
    catch (const std::ios_base::failure& e) {
        ::vo::show_error(instance(), &path, e.code(), "Failed to save {} /path/", type_name_lower_);
        return false;
    }

    return true;
}

bool config_manager::do_remove(const std::wstring& path)
{
    std::error_code ec;
    std::filesystem::remove(get_file_path(path), ec);
    if (ec) {
        show_error(xstr("Failed to remove "), &path, ec);
        return false;
    }

    // notification
    auto& style = instance()->style();
    instance()->notifications().create_note()
        << style.text_accent() << type_name_pascal_ << xstr(" removed ")
        << style.grey() << path
        << style.text() << xstr(".");

    return true;
}

bool config_manager::do_rename(const std::wstring& path, const std::wstring& new_path)
{
    std::error_code ec;
    std::filesystem::rename(get_file_path(path), get_file_path(new_path), ec);
    if (ec) {
        show_error(xstr("Failed to rename "), &path, ec);
        return false;
    }

    if (last_file_ == path)
        save_last_file(new_path);

    // notification
    auto& style = instance()->style();
    instance()->notifications().create_note()
        << style.text_accent() << type_name_pascal_ << xstr(" renamed ")
        << style.grey() << path
        << style.text() << std::u8string_view(u8" → ")
        << style.accent() << new_path
        << style.text() << xstr(".");

    return true;
}

void config_manager::show_error(const xstr& message, const std::wstring* name, const std::error_code& err) const
{
    name == nullptr ? ::vo::show_error(instance(), name, err, "{}", message) :
        ::vo::show_error(instance(), name, err, "{} {}", message, kPathMark);
}

void_end_