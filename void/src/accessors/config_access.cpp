#include <void/accessors/config.h>
#include <void/void.h>
#include <config/config.h>


void_begin_

void config::add_module(std::unique_ptr<config_module>&& module)
{
    instance()->config_->add_module(std::move(module));
}

std::string config::get_main_path() const
{
    return instance()->config_->get_main_path().string();
}

_config* config::get_config_instance() const
{
    return instance()->config_.get();
}

config_module::config_module(const xstr& name, std::uint32_t size)
    : name_(name),
      size_(size)
{
}

void_end_