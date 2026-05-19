#pragma once
#include <void/util/vobj.h>
#include <void/util/error.h>
#include <filesystem>
#include <string>
#include <void/util/xstr.h>


void_begin_

class config_manager : protected vobj {
protected:
    const std::wstring extension_;
    std::string type_name_lower_;  // e.g. config
    std::string type_name_pascal_; // e.g. Config
    std::filesystem::path path_;
    std::filesystem::path last_file_path_;
    std::wstring last_file_;

    inline static std::wstring_view kLastFileName = L"__last__";

public:
    config_manager(void_* instance, std::wstring&& extension, const std::string& type_name);

public:
    [[nodiscard]] bool do_init(const std::filesystem::path& path);

    bool load_last_file(const std::wstring*& out);
    bool save_last_file(const std::wstring& path);

    bool do_create_new(std::wstring& out);
    bool do_load(const std::wstring& path, std::vector<std::uint8_t>& out);
    bool do_save(const std::wstring& path, std::uint8_t* data, std::size_t size);
    bool do_remove(const std::wstring& name);
    bool do_rename(const std::wstring& name, const std::wstring& new_name);

    void show_error(const xstr& message, const std::wstring* name, const std::error_code& err = std::error_code()) const;

    [[nodiscard]] std::filesystem::path get_file_path(const std::wstring& path) {
        return path_ / (path + extension_);
    }
    [[nodiscard]] const auto& get_folder_path() const { 
        return path_;
    }
};

void_end_