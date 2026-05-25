#include <void/accessors/config.h>
#include <void/void.h>
#include <config/config.h>


void_begin_

std::size_t config::add_module(std::unique_ptr<config_module>&& module)
{
    return instance()->config_->add_module(std::move(module));
}

void config::remove_module(std::size_t module)
{
    instance()->config_->remove_module(module);
}

std::string config::get_main_path() const
{
    return instance()->config_->get_main_path().string();
}

_config* config::get_config_instance() const
{
    return instance()->config_.get();
}

void config::load_last_config()
{
    get_config_instance()->load_last_config();
}

void_end_