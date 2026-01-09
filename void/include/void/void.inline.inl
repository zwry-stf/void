#pragma once
#include "def.h"

// accessors
#include "accessors/resources.h"
#include "accessors/cursors.h"
#include "accessors/style.h"
#include "accessors/fonts.h"
#include "accessors/icons.h"
#include "accessors/render_target.h"
#include "accessors/background.h"
#include "accessors/options.h"
#include "accessors/callbacks.h"
#include "accessors/util.h"
#include "accessors/watermark.h"
#include "accessors/account.h"
#include "accessors/notifications.h"
#include "accessors/config.h"
#include "accessors/theme.h"

// contents
#include "contents/input/input.h"
#include "contents/input/input_owner.h"
#include "contents/input/input_owner_overlay.h"
#include "contents/widgets/textfield.h"

//
#include "util/time_point.h"
#include <r2/renderer.h>
#include <atomic>


// macros
#define _define_void_accessor(_class, _name)                     \
private:                                                         \
    ::vo::_class accessors_##_name##_ = ::vo::_class(this);      \
public:                                                          \
    _define_void_access(accessors_##_name##_, _name)

#define _define_void_access(_var, _name)          \
    auto& _name() { return _var; }                \
    const auto& _name() const { return _var; }