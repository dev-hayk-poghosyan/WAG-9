#include "UnigineGuidDatabase.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <string_view>

UnigineGuidDatabase::UnigineGuidDatabase(
    std::filesystem::path gteDataDir, std::filesystem::path worldDir)
    : m_gteDataDir(std::move(gteDataDir))
    , m_worldDir(std::move(worldDir))
{
}

std::string
UnigineGuidDatabase::guid(std::filesystem::path const& fileName) const
{
    std::filesystem::path relativeName = fileName;

    if (fileName.is_absolute())
    {
        if (auto const guidFromMeta = extractGuidFromFile(fileName))
        {
            return *guidFromMeta;
        }

        // In case guid is failed to be read from file on the absolute path
        // we can try to find it by relative paths and then in guid DBs.

        std::error_code ec;
        relativeName = std::filesystem::relative(fileName, m_worldDir, ec);

        if (ec || relativeName.empty())
        {
            relativeName = std::filesystem::relative(fileName, m_gteDataDir, ec);

            if (ec || relativeName.empty())
            {
                // We have no options left.
                return std::string();
            }
        }
    }
    else
    {
        if (std::filesystem::exists(m_worldDir / relativeName))
        {
            if (auto const guidFromMeta = extractGuidFromFile(m_worldDir / relativeName))
            {
                return *guidFromMeta;
            }
        }

        if (std::filesystem::exists(m_gteDataDir / relativeName))
        {
            if (auto const guidFromMeta = extractGuidFromFile(m_gteDataDir / relativeName))
            {
                return *guidFromMeta;
            }
        }
    }

    if (relativeName.extension() == metaExtension())
    {
        relativeName.replace_extension();
    }

    // In case of guid is failed read from  meta file, we should load unigine's guids.db in
    // the global space (gte/guids.db) overriden by the local db(world/guids.db).
    maybeLoadDatabases();

    assert(m_database);

    auto const& guidByFileName = m_database->get<RecordByFileName>();
    auto const it = guidByFileName.find(relativeName);

    return it == guidByFileName.end()
        ? std::string()
        : it->guid;
}

std::filesystem::path
UnigineGuidDatabase::fileName(std::string const& guid) const
{
    maybeLoadDatabases();

    auto const& fileNameByGuid = m_database->get<RecordByGuid>();
    auto const it = fileNameByGuid.find(guid);

    return it == fileNameByGuid.end()
        ? std::filesystem::path()
        : it->fileName;
}

std::unordered_map<std::string, std::filesystem::path>
UnigineGuidDatabase::worldGuids(std::filesystem::path const& relDir) const
{
    std::unordered_map<std::string, std::filesystem::path> result;

    std::error_code ec;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_worldDir / relDir, ec))
    {
        if (!entry.is_regular_file() || entry.path().extension() == metaExtension())
        {
            continue;
        }

        const auto relPath = std::filesystem::relative(entry.path(), m_worldDir, ec);
        auto relPathString = relPath.string();
        std::replace(std::begin(relPathString), std::end(relPathString), '\\', '/');

        result.insert({ guid(relPath), relPathString });
    }

    return result;
}

namespace
{

template<typename TContainer>
void fastLoadUnigineDb(std::filesystem::path dbPath, TContainer& container)
{
    auto const trim = [](std::string_view str)
    {
        static const char* const spaces = " \",\n\r";
        str.remove_prefix(std::min(str.find_first_not_of(spaces), str.size()));

        auto meaningAtRight = str.find_last_not_of(spaces);
        if (meaningAtRight != std::string::npos)
        {
            str.remove_suffix(str.size() - meaningAtRight - 1);
        }

        return str;
    };
    auto const fileSize = [](std::ifstream& ifs)
    {
        ifs.ignore(std::numeric_limits<std::streamsize>::max());
        const std::streamsize length = ifs.gcount();
        ifs.clear();
        ifs.seekg(0, std::ios_base::beg);

        return length;
    };

    std::ifstream ifs{ dbPath };

    if (!ifs)
        throw std::runtime_error("Failed to open the db");

    const std::streamsize length = fileSize(ifs);

    // Replace implementation read-all-parse-all by portion reading with incremental parsing.
    // Maybe the mapping file into RAM will improve perfomance.
    auto buffer = std::make_unique<char[]>(length);

    ifs.read(buffer.get(), length);
    if (ifs.gcount() != length)
    {
        throw std::runtime_error("Failed to read the whole file");
    }

    // It's a rough estimate of future records.
    // 140 = length_of_GUID(40) + average_path_length(100).
    container.reserve(2 * length / 140);

    std::streamsize idx = 0;
    while (idx < length)
    {
        if (buffer[idx++] != '"')
        {
            continue;
        }

        static const std::streamsize guidSize = 40;

        auto begin = idx;

        // Here we try to save some time by skiping the guid, instead of looping over it byte to byte. 
        std::streamsize end = idx + guidSize;
        std::streamsize colonIdx = end;

        // Read until eol. And remember colon's position to split a line by it into key=>value.
        while (buffer[end] != '\n' && buffer[end] != '\r' && end < length)
        {
            if (buffer[end] == ':')
                colonIdx = end;

            ++end;
        }

        std::string_view guid{ buffer.get() + begin, static_cast<size_t>(colonIdx - begin) };
        std::string_view name{ buffer.get() + colonIdx + 1, static_cast<size_t>(end - colonIdx - 1) };

        guid.remove_suffix(1);

        container.insert({ std::string(trim(guid)), std::string(trim(name)) });

        idx = end;
    }
}

};

void
UnigineGuidDatabase::buildFrom(std::filesystem::path const& dbDir, Records& records)
{
    if (!std::filesystem::exists(dbDir))
    {
        return;
    }

#ifdef UNIGINE_GUID_LOADING_STABLE_VERSION
    boost::property_tree::ptree tree;
    boost::property_tree::read_json((dbDir / guidsFileName()).string(), tree);

    for (auto const& [guid, filename] : tree)
    {
        records.insert({ guid, std::filesystem::path(filename.data()).make_preferred() });
    }
#else

    fastLoadUnigineDb(dbDir, records);

#endif // UNIGINE_GUID_LOADING_STABLE_VERSION
}

std::optional<std::string>
UnigineGuidDatabase::extractGuidFromFile(std::filesystem::path metaFileName)
{
    if (metaFileName.extension() != metaExtension())
    {
        metaFileName += metaExtension();
    }

    boost::property_tree::ptree tree;
    try
    {
        boost::property_tree::read_xml(metaFileName.string(), tree);
    }
    catch (...)
    {
        return std::nullopt;
    }

    if (auto asset = tree.get_child_optional("asset"))
    {
        if (auto guid = asset->get_child_optional("guid"))
        {
            return guid->data();
        }
    }

    return std::nullopt;
}

std::filesystem::path const&
UnigineGuidDatabase::guidsFileName()
{
    static const std::filesystem::path fn{ "guids.db" };
    return fn;
}

std::filesystem::path const&
UnigineGuidDatabase::metaExtension()
{
    static const std::filesystem::path ext{ ".meta" };
    return ext;
}

void
UnigineGuidDatabase::maybeLoadDatabases() const
{
    if (m_database)
        return;

    m_database.emplace();

    buildFrom(m_gteDataDir / guidsFileName(), *m_database);
    buildFrom(m_worldDir / guidsFileName(), *m_database);
}
