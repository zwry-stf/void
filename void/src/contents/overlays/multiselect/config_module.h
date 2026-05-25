#pragma once
#include <void/accessors/config.h>
#include <vector>
#include <assert.h>


void_begin_

class multiselect_config_module : public config_module {
private:
    std::vector<bool>* const value_;
    const std::vector<bool> default_value_;

public:
    multiselect_config_module(const string_token& name, std::vector<bool>* value)
        : config_module(name),
          value_(value),
          default_value_(value->begin(), value->end()) { }

public:
    virtual void reset() override {
        *value_ = default_value_;
    }
    virtual bool load(const std::uint8_t* buffer, std::uint32_t size) override {
        if (value_->size() * sizeof(bool) != size) {
            return false;
        }

        for (std::size_t i = 0; i < value_->size(); i++) {
            value_->at(i) = buffer[i] != 0u;
        }

        return true;
    }
    virtual void save(std::vector<std::uint8_t>& out_buffer) override {
        auto old_size = out_buffer.size();
        out_buffer.resize(
            old_size + static_cast<std::size_t>(value_->size() * sizeof(bool))
        );

        for (std::size_t i = 0; i < value_->size(); i++) {
            out_buffer[old_size + i] = value_->at(i) ? 1u : 0u;
        }
    }
};

void_end_