#include <void/accessors/resources.h>
#include <res/resource_data.h>
#include <resources/parser.h>


void_begin_

void resources::add_source(const std::uint8_t* data, std::size_t size)
{
    sources_.emplace_back(data, size);
}

loaded_resource resources::load_resource(int id)
{
    ::resources::parsed_file f;
    bool found = false;
    found = ::resources::parse(
        g_resources,
        sizeof(g_resources),
        id,
        f
    );
    if (!found) {
        for (auto& src : sources_) {
            found = ::resources::parse(
                src.data,
                src.size,
                id,
                f
            );

            if (found)
                break;
        }
    }
    if (!found)
        throw resource_error();

    return { f.data, f.size };
}

void_end_