#pragma once
#include <void/util/vobj.h>
#include <void/util/xstr.h>
#include <vector>
#include <memory>
#include <string>


void_begin_

class config_module {
protected:
	const xstr name_;
	const std::uint32_t size_;

public:
	config_module(const xstr& name, std::uint32_t size);

public:
	virtual void reset() = 0;
	virtual void load(const std::uint8_t* buffer) = 0;
	virtual void save(std::vector<std::uint8_t>& out_buffer) = 0;

public:
	[[nodiscard]] const auto& get_name() const noexcept {
		return name_;
	}
	[[nodiscard]] auto get_size() const noexcept {
		return size_;
	}
};

class config : protected vobj {
public:
	using vobj::vobj;

public:
	void add_module(std::unique_ptr<config_module>&& module);
	[[nodiscard]] std::string get_main_path() const;
	[[nodiscard]] class _config* get_config_instance() const;
};

void_end_