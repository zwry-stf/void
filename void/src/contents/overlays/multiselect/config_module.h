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
    multiselect_config_module(const xstr& name, std::vector<bool>* value)
        : config_module(name, static_cast<std::uint32_t>(value->size())),
          value_(value),
          default_value_(value->begin(), value->end()) { }

public:
    virtual void reset() override {
        *value_ = default_value_;
    }
    virtual void load(const std::uint8_t* buffer) override {
        assert(value_->size() == size_ / sizeof(bool));

        for (std::size_t i = 0; i < size_; i++) {
            value_->at(i) = buffer[i] != 0u;
        }
    }
    virtual void save(std::vector<std::uint8_t>& out_buffer) override {
        assert(value_->size() == size_ / sizeof(bool));

        auto old_size = out_buffer.size();
        out_buffer.resize(old_size + static_cast<std::size_t>(size_));

        for (std::size_t i = 0; i < size_; i++) {
            out_buffer[old_size + i] = value_->at(i) ? 1u : 0u;
        }
    }
};

void_end_