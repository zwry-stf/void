#pragma once
#include <void/util/vobj.h>
#include <void/util/xstr.h>
#include <vector>
#include <memory>
#include <string>
#include <assert.h>


void_begin_

class config_module {
protected:
    xstr name_;
    const std::uint32_t size_;
    const bool is_dynamic_;

public:
    config_module(const xstr& name, std::uint32_t size); // non dynamic
    config_module(const xstr& name); // dynamic

public:
    virtual void reset() = 0;
    virtual void load(const std::uint8_t* buffer) { 
        (void)buffer;
        assert(false);
    };
    virtual bool load_dynamic(const std::uint8_t* buffer, std::uint32_t size) {
        (void)buffer; (void)size;
        assert(false);
        return false;
    };
    virtual void save(std::vector<std::uint8_t>& out_buffer) = 0;

public:
    [[nodiscard]] const auto& get_name() const noexcept {
        return name_;
    }
    [[nodiscard]] auto get_size() const noexcept {
        return size_;
    }
    [[nodiscard]] auto is_dynamic() const noexcept {
        return is_dynamic_;
    }
    void set_name(const xstr& name) noexcept {
        name_ = name;
    }
};

class config : protected vobj {
public:
    using vobj::vobj;

public:
    std::size_t add_module(std::unique_ptr<config_module>&& module);
    void remove_module(std::size_t module);
    [[nodiscard]] std::string get_main_path() const;
    [[nodiscard]] class _config* get_config_instance() const;
    void load_last_config();
};

void_end_