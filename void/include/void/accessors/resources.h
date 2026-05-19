#pragma once
#include <void/util/vobj.h>
#include <resources/resources.h>
#include <cstdint>
#include <cstddef>
#include <vector>


void_begin_

class loaded_resource {
private:
    const std::uint8_t* data_;
    const std::size_t size_;

public:
    loaded_resource(const std::uint8_t* data, std::size_t size) noexcept
        : data_(data),
          size_(size) { }

public:
    [[nodiscard]] const auto* data() const noexcept {
        return data_;
    }
    [[nodiscard]] auto size() const noexcept {
        return size_;
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