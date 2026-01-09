#pragma once
#include <void/util/vobj.h>
#include <void/util/xstr.h>
#include <void/util/time_point.h>
#include <r2/renderer.h>
#include <functional>
#include <mutex>
#include <deque>


void_begin_

class notification_data {
public:
    r2::color color;
    bool text_width_calculated{ false };
    float text_width;

public:
    notification_data(const r2::color& c)
        : color(c) { }
    virtual ~notification_data() = default;

public:
    virtual void cache_text_size(r2::renderer2d& renderer) = 0;
    virtual void render(r2::renderer2d& renderer, const r2::vec2& pos, r2::color_u32 col) const = 0;
};

template <r2::unicode::string_like String>
class string_notification_data : public notification_data {
private:
    const String string_;

public:
    string_notification_data(String&& string, const r2::color& c)
        : notification_data(c),
          string_(std::move(string)) { }

public:
    virtual void cache_text_size(r2::renderer2d& renderer) override {
        if (renderer.get_text_width_strict<String>(string_, text_width))
            text_width_calculated = true;
        else
            text_width = renderer.get_text_width<String>(string_);
        text_width = std::ceil(text_width);
    }
    virtual void render(r2::renderer2d& renderer, const r2::vec2& pos, r2::color_u32 col) const override {
        renderer.add_text<String>(pos, col, string_);
    }
};

class notification_buider : protected vobj {
private:
	r2::color current_color_;
	std::vector<std::unique_ptr<notification_data>> data_;

public:
	notification_buider(void_* instance);
    ~notification_buider();

    notification_buider(const notification_buider&) = delete;
    notification_buider& operator=(const notification_buider&) = delete;
    notification_buider(notification_buider&&) = default;
    notification_buider& operator=(notification_buider&&) = default;

public:
    notification_buider& operator<<(const r2::color& color) {
        current_color_ = color;
        return *this;
    }

    template <r2::unicode::string_like String>
    notification_buider& operator<<(const String& s) {
        String owned = s;
        return (*this) << std::move(owned);
    }

    template <r2::unicode::string_like String>
    notification_buider& operator<<(String&& s) {
        using stored_t = std::remove_cvref_t<String>;
        data_.push_back(
            std::make_unique<string_notification_data<stored_t>>(
                std::move(s),
                current_color_
            )
        );
        return *this;
    }
};

class notifications : protected vobj {
private:
    struct notification {
        notification(const time_point& time, std::vector<std::unique_ptr<notification_data>>&& data)
            : creation_time(time),
              data(std::move(data)) { }

        time_point creation_time;
        float pos_y{ -1.f };
        std::vector<std::unique_ptr<notification_data>> data;
    };

    std::mutex mutex_;
    std::deque<notification> notifications_;

public:
    using vobj::vobj;

public:
    notification_buider create_notification();
    notification_buider create_note();
    notification_buider create_error();

    void add_notification(std::vector<std::unique_ptr<notification_data>>&& data);

    void render();

    void on_scale_changed();
};

void_end_