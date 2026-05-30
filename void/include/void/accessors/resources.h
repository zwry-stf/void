#pragma once
#include <void/util/vobj.h>
#include <resources/resources.h>
#include <cstdint>
#include <cstddef>
#include <vector>


void_begin_

class loaded_resource {
private:
    std::vector<std::uint8_t> data_;

public:
    loaded_resource(std::vector<std::uint8_t> data) noexcept
        : data_(std::move(data)) { }

public:
    [[nodiscard]] const auto* data() const noexcept {
        return data_.data();
    }
    [[nodiscard]] auto size() const noexcept {
        return data_.size();
    }
    [[nodiscard]] std::vector<std::uint8_t> move() noexcept {
        return std::move(data_);
    }
    [[nodiscard]] std::vector<std::uint8_t>& vec() noexcept {
        return data_;
    }
    [[nodiscard]] const std::vector<std::uint8_t>& vec() const noexcept {
        return data_;
    }
};

class resources : protected vobj {
private:
    struct resource_source {
        const std::uint8_t* data;
        const std::size_t size;
    };
    std::vector<resource_source> sources_;

public:
    using vobj::vobj;

public:
    void add_source(const std::uint8_t* data, std::size_t size);
    [[nodiscard]] loaded_resource load_resource(int id);
};

void_end_