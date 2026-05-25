#pragma once
#include <void/util/vobj.h>
#include <void/util/string_token.h>

#include <vector>
#include <memory>
#include <string>
#include <cassert>


void_begin_

class config_module {
protected:
    string_token name_;

public:
    config_module(const string_token& name)
        : name_(name) { }

public:
    virtual void reset() = 0;
    virtual bool load(const std::uint8_t* buffer, std::uint32_t size) = 0;
    virtual void save(std::vector<std::uint8_t>& out_buffer) = 0;

public:
    [[nodiscard]] string_token get_name() const noexcept {
        return name_;
    }
    void set_name(const string_token& name) noexcept {
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