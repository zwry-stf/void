#pragma once
#include <void/def.h>
#include <void/util/xstr.h>
#include <cstdint>
#include <string>


void_begin_

enum class error_code : std::int32_t {
    background_init,
    background_overlay_init,
    render_target_init,
    load_icon,
    config_init,
};

class error {
private:
    error_code code_;
    std::int32_t error_;
    std::int32_t detail_;

public:
    error(error_code code, std::int32_t error, std::int32_t detail) noexcept
        : code_(code), error_(error), detail_(detail) { }

    error(error_code code) noexcept
        : error(code, -1, 0) { }

public:
    bool operator==(const error& v) const {
        return code_ == v.code_;
    }

public:
    [[nodiscard]] xstr get_error_message() const {
        xstr error;

        switch (code_) {
        case error_code::background_init: error = "Vane background initialization failed."; break;
            /*
        case error_code::ErrorFontBuild:        error = "Vane font build failed."; break;
        case error_code::ErrorIconInit:         error = "Vane icon initialization failed."; break;
        case error_code::ErrorIconCreate:       error = "Vane icon creation failed."; break;
        case error_code::ErrorCursorInit:       error = "Vane cursor initialization failed."; break;
        case error_code::ErrorRenderTargetInit: error = "Vane render target creation failed."; break;
        case error_code::BackgroundInit:        error = "Vane background initialization failed."; break;
        case error_code::ConfigInit:            error = "Vane config initialization failed."; break;*/

        default: error = "Unknown error";
        }

        if (error_ != -1) {
            error.append_safe(xstr(" ( "));
            error.append_safe(std::to_string(error_).c_str());
            if (detail_ != 0) {
                error.append_safe(xstr(" - "));
                error.append_safe(std::to_string(detail_).c_str());
            }
            error.append_safe(xstr(" )"));
        }

        return error;
    }
};

void_end_