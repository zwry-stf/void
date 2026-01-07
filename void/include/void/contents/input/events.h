#pragma once
#include "message_event.h"
#include "input_receiver.h"
#include <assert.h>
#include <concepts>
#include <algorithm>


void_begin_

class render_input {
private:
    const std::int64_t hovered_;
    const std::int64_t selected_;

public:
    render_input(auto hovered, auto selected) noexcept
        : hovered_(hovered),
          selected_(selected) { }
    
public:
    [[nodiscard]] bool is_hovered(const input_receiver* r) const noexcept {
        return hovered_ >= r->id_min_ && hovered_ < r->id_max_;
    }
    [[nodiscard]] bool is_selected(const input_receiver* r) const noexcept {
        return selected_ >= r->id_min_ && selected_ < r->id_max_;
    }
    template <std::integral T>
    [[nodiscard]] bool is_hovered(const input_receiver* r, T off) const noexcept {
        assert((r->id_max_ - r->id_min_) > static_cast<std::int64_t>(off));
        assert(static_cast<std::int64_t>(off) >= 0);
        return hovered_ == r->id_min_ + static_cast<std::int64_t>(off);
    }
    template <typename T>
        requires(std::is_enum_v<T>)
    [[nodiscard]] bool is_hovered(const input_receiver* r, T off) const noexcept {
        return is_hovered(r, std::to_underlying(off));
    }
    template <std::integral T>
    [[nodiscard]] bool is_selected(const input_receiver* r, T off) const noexcept {
        assert((r->id_max_ - r->id_min_) > static_cast<std::int64_t>(off));
        assert(static_cast<std::int64_t>(off) >= 0);
        return selected_ == r->id_min_ + static_cast<std::int64_t>(off);
    }
    template <typename T>
        requires(std::is_enum_v<T>)
    [[nodiscard]] bool is_selected(const input_receiver* r, T off) const noexcept {
        return is_selected(r, std::to_underlying(off));
    }
    template <std::integral T>
    [[nodiscard]] bool is_range_hovered(const input_receiver* r, T start, T end) const noexcept {
        assert(start < end);
        assert((r->id_max_ - r->id_min_) > start);
        assert(static_cast<std::int64_t>(start) >= 0);
        assert((r->id_max_ - r->id_min_) > end);
        assert(static_cast<std::int64_t>(end) >= 0);
        return hovered_ >= r->id_min_ + static_cast<std::int64_t>(start) && 
               hovered_ <= r->id_min_ + static_cast<std::int64_t>(end);
    }
    template <typename T>
        requires(std::is_enum_v<T>)
    [[nodiscard]] bool is_range_hovered(const input_receiver* r, T start, T end) const noexcept {
        return is_range_hovered(r, std::to_underlying(start), std::to_underlying(end));
    }
    template <std::integral T>
    [[nodiscard]] bool is_range_selected(const input_receiver* r, T start, T end) const noexcept {
        assert(start < end);
        assert((r->id_max_ - r->id_min_) > start);
        assert(static_cast<std::int64_t>(start) >= 0);
        assert((r->id_max_ - r->id_min_) > end);
        assert(static_cast<std::int64_t>(end) >= 0);
        return selected_ >= r->id_min_ + static_cast<std::int64_t>(start) &&
               selected_ <= r->id_min_ + static_cast<std::int64_t>(end);
    }
    template <typename T>
        requires(std::is_enum_v<T>)
    [[nodiscard]] bool is_range_selected(const input_receiver* r, T start, T end) const noexcept {
        return is_range_selected(r, std::to_underlying(start), std::to_underlying(end));
    }
};

class input_base {
private:
    const message_event* event_{};
    std::int64_t* const selected_ptr_;
    std::int64_t* const hovered_ptr_;

public:
    input_base(auto& hovered, auto& selected) noexcept
        : hovered_ptr_(&hovered), 
          selected_ptr_(&selected) { }

public:
    [[nodiscard]] const message_event& event() const noexcept { 
        assert(event_ != nullptr);
        return *event_; 
    }
    void set_event(const message_event& event) noexcept {
        event_ = &event;
    }
    
public:
    [[nodiscard]] bool is_hovered(const input_receiver* r) const noexcept {
        return *hovered_ptr_ >= r->id_min_ && *hovered_ptr_ < r->id_max_;
    }
    [[nodiscard]] bool is_selected(const input_receiver* r) const noexcept {
        return *selected_ptr_ >= r->id_min_ && *selected_ptr_ < r->id_max_;
    }
    template <std::integral T>
    [[nodiscard]] bool is_hovered(const input_receiver* r, T off) const noexcept {
        assert((r->id_max_ - r->id_min_) > static_cast<std::int64_t>(off));
        assert(static_cast<std::int64_t>(off) >= 0);
        return *hovered_ptr_ == r->id_min_ + static_cast<std::int64_t>(off);
    }
    template <typename T>
        requires(std::is_enum_v<T>)
    [[nodiscard]] bool is_hovered(const input_receiver* r, T off) const noexcept {
        return is_hovered(r, std::to_underlying(off));
    }
    template <std::integral T>
    [[nodiscard]] bool is_selected(const input_receiver* r, T off) const noexcept {
        assert((r->id_max_ - r->id_min_) > static_cast<std::int64_t>(off));
        assert(static_cast<std::int64_t>(off) >= 0);
        return *selected_ptr_ == r->id_min_ + static_cast<std::int64_t>(off);
    }
    template <typename T>
        requires(std::is_enum_v<T>)
    [[nodiscard]] bool is_selected(const input_receiver* r, T off) const noexcept {
        return is_selected(r, std::to_underlying(off));
    }
    template <std::integral T>
    [[nodiscard]] bool is_range_hovered(const input_receiver* r, T start, T end) const noexcept {
        assert(start < end);
        assert((r->id_max_ - r->id_min_) > start);
        assert(static_cast<std::int64_t>(start) >= 0);
        assert((r->id_max_ - r->id_min_) > end);
        assert(static_cast<std::int64_t>(end) >= 0);
        return *hovered_ptr_ >= r->id_min_ + static_cast<std::int64_t>(start) && 
               *hovered_ptr_ <= r->id_min_ + static_cast<std::int64_t>(end);
    }
    template <typename T>
        requires(std::is_enum_v<T>)
    [[nodiscard]] bool is_range_hovered(const input_receiver* r, T start, T end) const noexcept {
        return is_range_hovered(r, std::to_underlying(start), std::to_underlying(end));
    }
    template <std::integral T>
    [[nodiscard]] bool is_range_selected(const input_receiver* r, T start, T end) const noexcept {
        assert(start < end);
        assert((r->id_max_ - r->id_min_) > start);
        assert(static_cast<std::int64_t>(start) >= 0);
        assert((r->id_max_ - r->id_min_) > end);
        assert(static_cast<std::int64_t>(end) >= 0);
        return *selected_ptr_ >= r->id_min_ + static_cast<std::int64_t>(start) &&
               *selected_ptr_ <= r->id_min_ + static_cast<std::int64_t>(end);
    }
    template <typename T>
        requires(std::is_enum_v<T>)
    [[nodiscard]] bool is_range_selected(const input_receiver* r, T start, T end) const noexcept {
        return is_range_selected(r, std::to_underlying(start), std::to_underlying(end));
    }
    [[nodiscard]] bool nothing_selected() const noexcept {
        return *selected_ptr_ == -1;
    }

public:
    void set_hovered(const input_receiver* r) const noexcept {
        assert(r->id_max_ - r->id_min_ == 1);
        *hovered_ptr_ = r->id_min_;
    }
    void set_selected(const input_receiver* r) const noexcept {
        assert(r->id_max_ - r->id_min_ == 1);
        *selected_ptr_ = r->id_min_;
    }
    template <std::integral T>
    void set_hovered(const input_receiver* r, T off) const noexcept {
        assert((r->id_max_ - r->id_min_) > static_cast<std::int64_t>(off));
        *hovered_ptr_ = r->id_min_ + static_cast<std::int64_t>(off);
    }
    template <typename T>
        requires(std::is_enum_v<T>)
    void set_hovered(const input_receiver* r, T off) const noexcept {
        set_hovered(r, std::to_underlying(off));
    }
    template <std::integral T>
    void set_selected(const input_receiver* r, T off) const noexcept {
        assert((r->id_max_ - r->id_min_) > static_cast<std::int64_t>(off));
        *selected_ptr_ = r->id_min_ + static_cast<std::int64_t>(off);
    }
    template <typename T>
        requires(std::is_enum_v<T>)
    void set_selected(const input_receiver* r, T off) const noexcept {
        set_selected(r, std::to_underlying(off));
    }
    void clear_selected() const noexcept {
        *selected_ptr_ = -1;
    }
};

void_end_