#pragma once
#include <void/def.h>
#include <cstdint>
#include <cstddef>
#include <concepts>
#include <assert.h>
#include <cctype>
#include <algorithm>


void_begin_

template <typename T>
concept xstr_character =
    std::same_as<T, char> ||
    std::same_as<T, signed char> ||
    std::same_as<T, unsigned char> ||
    std::same_as<T, wchar_t> ||
    std::same_as<T, char8_t> ||
    std::same_as<T, char16_t> ||
    std::same_as<T, char32_t>;

namespace fnv1a {
    consteval std::uint64_t fnv1a_64(const char* s) noexcept {
        std::uint64_t h = 14695981039346656037ull;
        for (std::size_t i = 0; s[i] != '\0'; ++i) {
            h ^= static_cast<unsigned char>(s[i]);
            h *= 1099511628211ull;
        }
        return h;
    }

    consteval std::uint64_t splitmix64_next(std::uint64_t& x) noexcept {
        x += 0x9E3779B97F4A7C15ull;
        std::uint64_t z = x;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        return z ^ (z >> 31);
    }
}

template <xstr_character Char, std::size_t Count>
class basic_xstr {
private:
    std::size_t length_;
    Char key_;
    Char chars_[Count];

    inline static constexpr Char kNullTerminator = Char(0);

public:
    inline static constexpr std::size_t kMaxSize = Count;

private:
    static consteval Char to_character(std::uint64_t v) noexcept {
        if constexpr (std::same_as<Char, char8_t>) {
            return static_cast<char8_t>(static_cast<std::uint8_t>(v));
        }
        else if constexpr (std::same_as<Char, signed char>) {
            return static_cast<signed char>(static_cast<std::uint8_t>(v));
        }
        else if constexpr (std::same_as<Char, unsigned char>) {
            return static_cast<unsigned char>(static_cast<std::uint8_t>(v));
        }
        else if constexpr (std::same_as<Char, char>) {
            return static_cast<char>(static_cast<unsigned char>(v));
        }
        else {
            using U = std::make_unsigned_t<Char>;
            constexpr std::size_t kBytes = sizeof(Char);
            U out = 0;
            for (std::size_t i = 0; i < kBytes; ++i) {
                out |= (static_cast<U>((v >> (8 * i)) & 0xFFull) << (8 * i));
            }
            return static_cast<Char>(out);
        }
    }

    consteval static Char generate_key_const(std::uint64_t counter) noexcept {
        std::uint64_t s = 0;

        s ^= fnv1a::splitmix64_next(counter);

        s ^= fnv1a::fnv1a_64(__FILE__);
        s ^= fnv1a::fnv1a_64(__DATE__);
        s ^= fnv1a::fnv1a_64(__TIME__);
        s ^= (static_cast<std::uint64_t>(__LINE__) << 32);
        s ^= static_cast<std::uint64_t>(__COUNTER__) * 0xD6E8FEB86659FD93ull;

        const std::uint64_t r = fnv1a::splitmix64_next(s);
        return to_character(r);
    }

    consteval void init_bytes() noexcept {
        for (std::size_t i = 0; i < kMaxSize; i++) {
            chars_[i] = generate_key_const(static_cast<std::uint64_t>(i));
        }
    }

    template<xstr_character, std::size_t>
    friend class basic_xstr;
    friend class iterator;

public:
    consteval basic_xstr() noexcept
        : length_(0u),
          key_(generate_key_const(0u)),
          chars_() {
        init_bytes();
    }

    template <std::size_t N>
    consteval basic_xstr(const Char(&text)[N]) noexcept
        : basic_xstr() {
        static_assert(N > 0u);
        length_ = N - 1u;

        for (std::size_t i = 0u; i < N - 1u; i++) {
            chars_[i] = text[i] ^ key_;
        }
    }

    constexpr basic_xstr(const basic_xstr<Char, Count>&) = default;
    constexpr basic_xstr(basic_xstr<Char, Count>&&) = default;
    constexpr basic_xstr<Char, Count>& operator=(const basic_xstr<Char, Count>&) = default;
    constexpr basic_xstr<Char, Count>& operator=(basic_xstr<Char, Count>&&) = default;

    template <std::size_t N>
    basic_xstr<Char, Count>& operator=(const basic_xstr<Char, N>& v) noexcept {
        if (reinterpret_cast<const void*>(this) == reinterpret_cast<const void*>(&v))
            return *this;

        assert(v.length_ <= kMaxSize);

        key_ = v.key_;
        length_ = v.length_;

        for (std::size_t i = 0u; i < length_; i++)
            chars_[i] = v.chars_[i];

        return *this;
    }

public:
    class iterator {
    private:
        std::size_t pos_;
        basic_xstr<Char, Count>* const ref_;

    public:
        iterator(std::size_t pos, basic_xstr<Char, Count>* ref) noexcept
            : pos_(pos),
              ref_(ref) { }

    public:
        [[nodiscard]] Char operator*() const noexcept {
            return ref_->chars_[pos_] ^ ref_->key_;
        }
        [[nodiscard]] explicit operator Char() const noexcept {
            return ref_->chars_[pos_] ^ ref_->key_;
        }
        iterator& operator++() noexcept {
            assert((pos_ < basic_xstr<Char, Count>::kMaxSize));
            pos_++;
            return *this;
        }
        iterator& operator--() noexcept {
            assert(pos_ > 0u);
            pos_--;
            return *this;
        }
        iterator& operator=(Char v) noexcept {
            ref_->chars_[pos_] = v ^ ref_->key_;
        }
    };

public:
    template <std::size_t N>
    inline constexpr basic_xstr<Char, Count>& operator+=(const basic_xstr<Char, N>& other) {
        assert(static_cast<std::uint16_t>(length_) + static_cast<std::uint16_t>(other.length()) <=
            kMaxSize);

        for (std::size_t i = 0u; i < other.length(); i++)
            chars_[length_ + i] = other[i] ^ key_;

        length_ += other.length();

        return *this;
    }

    template <std::size_t N>
    [[nodiscard]] inline constexpr basic_xstr<Char, Count> operator+(const basic_xstr<Char, N>& other) const {
        basic_xstr ret = *this;
        ret += other;

        return ret;
    }

    inline constexpr basic_xstr<Char, Count>& operator+=(Char c) {
        assert(length_ + 1u <= kMaxSize);

        chars_[length_++] = c ^ key_;

        return *this;
    }

    [[nodiscard]] inline constexpr basic_xstr<Char, Count> operator+(Char c) const {
        basic_xstr ret = *this;
        ret += c;

        return ret;
    }

public:
    void append(const Char* str) {
        while (*str != kNullTerminator) {
            assert(length_ + 1u <= kMaxSize);
            chars_[length_++] = static_cast<Char>(*str ^ key_);
            ++str;
        }
    }

    void append_safe(const Char* str) {
        while (*str != kNullTerminator) {
            if (length_ == kMaxSize)
                return;

            chars_[length_++] = static_cast<Char>(*str ^ key_);
            ++str;
        }
    }

    template <std::size_t N>
    void append_safe(const basic_xstr<Char, N>& other) {
        std::size_t length = other.length_;
        if (length_ + length > kMaxSize) {
            length = kMaxSize - length_;
        }

        for (std::size_t i = 0u;
            i < length; i++) {
            (*this) += other[i];
        }
    }

    [[nodiscard]] inline bool equals(const Char* other) const {
        for (std::size_t i = 0u; i < length_; i++) {
            Char c = chars_[i] ^ key_;
            if (c != other[i])
                return false;
        }

        if (other[length_] != kNullTerminator)
            return false;

        return true;
    }

public:
    template <std::integral T>
    [[nodiscard]] iterator operator[](T index) noexcept {
        return iterator(static_cast<std::size_t>(index), this);
    }

    template <std::integral T>
    [[nodiscard]] Char operator[](T index) const noexcept {
        return chars_[index] ^ key_;
    }

    template <std::integral T>
    [[nodiscard]] Char at(T index) const noexcept {
        return (*this)[index];
    }

    [[nodiscard]] inline bool operator==(const Char* other) const {
        return equals(other);
    }

    template <std::size_t N>
    [[nodiscard]] inline constexpr bool operator==(const basic_xstr<Char, N>& other) const {
        if (length_ != other.length_)
            return false;

        for (std::size_t i = 0u; i < length_; i++) {
            Char c = chars_[i] ^ key_;
            if (c != other[i])
                return false;
        }

        return true;
    }

    [[nodiscard]] std::size_t length() const noexcept {
        return length_;
    }

    [[nodiscard]] bool empty() const noexcept {
        return length_ == 0u;
    }

public:
    inline static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    template <std::size_t N>
    [[nodiscard]] inline constexpr std::size_t find(const basic_xstr<Char, N>& str) const {
        const auto pat_len = str.length();
        const auto txt_len = length();
        if (pat_len == 0u)
            return 0u;

        if (pat_len > txt_len)
            return npos;

        for (std::size_t i = 0u; i <= static_cast<std::size_t>(txt_len - pat_len); ++i) {
            bool match = true;
            for (std::size_t j = 0u; j < pat_len; ++j) {
                const Char c = chars_[i + j] ^ key_;
                if (c != str[j]) {
                    match = false;
                    break;
                }
            }
            if (match)
                return i;
        }
        return npos;
    }

    [[nodiscard]] inline constexpr std::size_t find(const Char c, std::size_t off) const {
        for (std::size_t i = off; i < length(); ++i) {
            const Char _c = chars_[i] ^ key_;
            if (_c == c)
                return i;
        }
        return npos;
    }

    // case insensitive string find funcion that assumes that the input string is already lower case
    template <std::size_t N>
    [[nodiscard]] inline constexpr std::size_t find_ignore_case_f(const basic_xstr<Char, N>& str) const {
        const auto pat_len = str.length();
        const auto txt_len = length();
        if (pat_len == 0u)
            return 0u;

        if (pat_len > txt_len)
            return npos;

        for (std::size_t i = 0u; i <= txt_len - pat_len; ++i) {
            bool match = true;
            for (std::size_t j = 0u; j < pat_len; ++j) {
                const Char c = static_cast<Char>(
                    std::tolower(static_cast<int>(chars_[i + j] ^ key_))
                );
                if (c != str[j]) {
                    match = false;
                    break;
                }
            }
            if (match)
                return i;
        }
        return npos;
    }

    template <std::size_t N>
    void get(Char(&out)[N]) const noexcept {
        get(&out[0], N * sizeof(Char));
    }

    void get(Char* out, std::size_t size) const noexcept {
        if (size == 0u)
            return;

        std::size_t len2 = (size / sizeof(Char)) - 1u;
        std::size_t len = length_ > len2 ? len2 : length_;

        for (std::size_t i = 0u; i < len; i++)
            out[i] = chars_[i] ^ key_;

        out[len] = kNullTerminator;
    }
};

using xstr  = basic_xstr<char,    64u>;
using wxstr = basic_xstr<wchar_t, 64u>;

void_end_