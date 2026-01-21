#pragma once
#include <void/util/vobj.h>
#include <void/util/xstr.h>


void_begin_

class base_builder_object : protected vobj {
private:
    class menu_builder* const builder_;

public:
    base_builder_object(void_* instance, class menu_builder* builder) noexcept
        : vobj(instance),
        builder_(builder) {
    }

protected:
    [[nodiscard]] auto* builder() const noexcept {
        return builder_;
    }

    [[nodiscard]] const xstr& get_last_child_name() const;
    [[nodiscard]] const xstr& get_last_group_name() const;
    void set_last_child_name(const xstr& s);
    void set_last_group_name(const xstr& s);
};

void_end_