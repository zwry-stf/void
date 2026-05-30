#pragma once
#include <void/def.h>

#include <cstdint>
#include <cstddef>
#include <string_view>


void_begin_

class string_token {
private:
	std::uint32_t hash_;

private:
	[[nodiscard]] static constexpr std::uint32_t calc_hash(const char* text, std::size_t len) noexcept {
		std::uint32_t hash = 0x811C9DC5u;
		for (std::size_t i = 0; i < len; ++i) {
			hash ^= static_cast<std::uint32_t>(static_cast<unsigned char>(text[i]));
			hash *= 0x01000193u;
		}
		return hash;
	}

	[[nodiscard]] static constexpr std::uint32_t combine_hashes(std::uint32_t h1, std::uint32_t h2) noexcept {
		h1 ^= h2 + 0x9e3779b9u + (h1 << 6u) + (h1 >> 2u);
		return h1;
	}

public:
	template <std::size_t N>
	consteval string_token(const char(&text)[N]) noexcept 
		: hash_(calc_hash(text, N - 1)) { }
	string_token(const std::string_view& text) noexcept 
		: hash_(calc_hash(text.data(), text.length())) { }

	constexpr explicit string_token(std::uint32_t hash) noexcept
		: hash_(hash) { }
	constexpr string_token() noexcept
		: string_token(0) { }

public:
	[[nodiscard]] string_token combine(const string_token& t) const noexcept {
		return string_token(
			combine_hashes(hash_, t.hash_)
		);
	}

public:
	[[nodiscard]] std::uint32_t hash() const noexcept {
		return hash_;
	}
	[[nodiscard]] bool operator==(const string_token& o) const noexcept {
		return hash_ == o.hash_;
	}
};

void_end_