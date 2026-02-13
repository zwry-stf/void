#pragma once
#include <void/accessors/config.h>
#include <vector>
#include <assert.h>
#include <void/util/xstr.h>
#include <void/contents/widgets/list_options.h>


void_begin_

class list_name_config_module : public config_module {
private:
    const std::size_t default_value_;
    list_options* const list_options_;
    std::size_t* const value_;

    static_assert(std::is_trivially_copyable_v<xstr>);
    static_assert(std::is_trivially_destructible_v<xstr>);
    static_assert(std::is_standard_layout_v<xstr>);

public:
    list_name_config_module(const xstr& name, std::size_t* value, list_options* options)
        : config_module(name, static_cast<std::uint32_t>(sizeof(xstr))),
          list_options_(options),
          value_(value),
          default_value_(*value) { }

public:
    virtual void reset() {
        *value_ = default_value_;
    }
    virtual void load(const std::uint8_t* buffer, std::uint32_t size) {
        assert(size == size_);
        (void)size;
        xstr res;
        std::memcpy(
            &res,
            buffer,
            sizeof(xstr)
        );

        if (res.empty() ||
            res.length() > xstr::kMaxSize) {
            return;
        }

        const auto options_size = list_options_->size();
        for (std::size_t i = 0u; i < options_size; i++) {
            if (list_options_->element_safe(i) == res) {
                *value_ = i;
                break;
            }
        }
    }
    virtual void save(std::vector<std::uint8_t>& out_buffer) {
        const auto& res = list_options_->element_safe(*value_);

        const auto old_size = out_buffer.size();
        out_buffer.resize(old_size + sizeof(xstr));
        std::memcpy(
            out_buffer.data() + old_size,
            &res,
            sizeof(res)
        );
    }
};

void_end_