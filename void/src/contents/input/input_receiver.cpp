#include <void/contents/input/input_receiver.h>
#include <void/contents/input/input_owner.h>
#include <void/contents/input/input_owner_overlay.h>


void_begin_

input_receiver::input_receiver(input_owner* owner, std::int32_t num) noexcept
    : input_owner_(owner),
      id_min_(owner->create_ids(num)),
      id_max_(id_min_ + num)
{
}

void input_receiver::resize_input(std::int32_t num) noexcept
{
    id_min_ = input_owner_->resize(id_min_, id_max_, num);

    id_max_ = id_min_ + num;
}

input_receiver_overlay::input_receiver_overlay(input_owner_overlay* owner) noexcept
    : id_(owner->create_id())
{
}

void_end_