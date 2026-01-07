#pragma once
#include "config_manager.h"
#include <void/accessors/config.h>


void_begin_

struct default_config_module : public config_module {

};

class config_drawable;
class _config : public config_manager {
private:
	std::filesystem::path main_path_;
	std::vector<std::unique_ptr<config_module>> modules_;
	std::vector<std::unique_ptr<config_drawable>> configs_;

public:
	_config(void_* instance);
	~_config();

	inline static constexpr std::string_view kConfigExtension = ".cfg";

public:
	void init();
	void destroy();

public:
	void add_module(std::unique_ptr<config_module>&& module);

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