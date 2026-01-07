#include <void/accessors/resources.h>
#include <res/resource_data.h>
#include <resources/parser.h>


void_begin_

loaded_resource resources::load_resource(int id)
{
    ::resources::parsed_file f;
    if (!::resources::parse(g_resources, sizeof(g_resources), id, f))
        throw resource_error();

    return { f.data, f.size };
}

void_end_