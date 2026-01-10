#pragma once
#include "config_manager.h"
#include <void/accessors/config.h>


void_begin_

template <typename T>
class default_config_module : public config_module {
private:
	T* const value_;
	const T default_value_;

public:
	default_config_module(const xstr& name, T* value)
		: config_module(name, static_cast<std::uint32_t>(sizeof(T))),
		  value_(value), 
		  default_value_(*value) { }

public:
	virtual void reset() override {
		*value_ = default_value_;
	}
	virtual void load(const std::uint8_t* data) override {
		std::memcpy(value_, data, sizeof(T));
	}
	virtual void save(std::vector<std::uint8_t>& out_buffer) override {
		auto old_size = out_buffer.size();
		out_buffer.resize(old_size + sizeof(T));

		std::memcpy(out_buffer.data() + old_size, value_, sizeof(T));
	}
};

class config_drawable;
class _config : public config_manager {
private:
	std::filesystem::path main_path_;
	std::vector<std::unique_ptr<config_module>> modules_;
	std::vector<std::size_t> freed_modules_;
	std::vector<std::unique_ptr<config_drawable>> configs_;

public:
	_config(void_* instance);
	~_config();

	inline static constexpr std::string_view kConfigExtension = ".cfg";

public:
	void init();
	void destroy();

public:
	inline static constexpr std::size_t kInvalidModuleId = static_cast<std::size_t>(-1);
	std::size_t add_module(std::unique_ptr<config_module>&& module);
	void remove_module(std::size_t module);

	[[nodiscard]] const auto& get_main_path() {
		return main_path_; 
	}

public:
	bool create_new();
	bool load(const std::wstring& name, bool first = false);
	bool save(const std::wstring& name, config_drawable* drawable);
	bool remove(const std::wstring& name, config_drawable* drawable);
	bool rename(const std::wstring& name, const std::wstring& new_name);

	bool refresh_configs(std::int32_t& selected_config);

	void update(std::int32_t& selected_config);

	friend class config_tab;
};

void_end_