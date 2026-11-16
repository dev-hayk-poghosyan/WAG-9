#include "GetExecutableDir.h"

#include <boost/dll/runtime_symbol_info.hpp>

boost::filesystem::path getExecutableDir()
{
    boost::filesystem::path path(boost::dll::program_location());
    path.remove_filename(); // Remove executable name.
    
    return path;
}
