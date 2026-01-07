#pragma once
#include <void/def.h>
#include <void/util/xstr.h>
#include <r2/renderer_definitions.h>
#include <vector>


void_begin_

class list_options {
public:
    inline static constinit xstr kNone = xstr("None");

public:
    virtual ~list_options() = default;

public:
    virtual bool is_dynamic() const noexcept = 0;
    virtual std::size_t size() const noexcept = 0;
    virtual const xstr& element(std::size_t index) const = 0;
    virtual const xstr& element_safe(std::size_t index) const = 0;

public:
    template <typename T, std::integral I>
    static list_options* create_member_constant(
        const T* data, I size, xstr T::* member);

    template <typename T>
    static list_options* create_member_dynamic(
        T** data, std::size_t* size, xstr T::* member);

    template <std::integral I>
    static list_options* create_constant(
        const xstr* data, I size);

    template <std::size_t N>
    static list_options* create_constant(
        const xstr(&data)[N]);

    template <typename T>
    static list_options* create_vector_member_constant(
        const std::vector<T>& data, xstr T::* member);

    template <typename T>
    static list_options* create_vector_member_dynamic(
        const std::vector<T>** data, xstr T::* member);
};

void_end_

#include "list_options.inline.inl"