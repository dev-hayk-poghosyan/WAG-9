#pragma once

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>

#include <filesystem>

class UnigineGuidDatabase
{
public:
    /**
     * @param gteDataDir GTE Data directory(e.g. d:\GlobalTerrainEditor-release\data).
     * @param worldDir  explicit world dir, mount/symlink resolved (e.g. e:\Worlds\Ru15).
     */
    UnigineGuidDatabase(std::filesystem::path gteDataDir, std::filesystem::path worldDir);

    /**
     * Retrieves guid for specified @p fileName.
     * @param fileName Can be absolute or relative to the GTE data or the world dir.
     * if @p fileName is absolute path: guid is tried to extract directly from it.
     * On failure @p fileName is converted to relative if possible to gte data or world dir, and then
     * processes as a relative path.
     *
     * if @p fileName is relative we try found meta file for it in world dir and then in GTE dir.
     * if that fails we load unigine guid db(if it's not loaded already) and search in it.
     * 
     * If @p fileName is a meta file(i.e. "c:/gte/properties/some.prop.meta") guid is retrived for
     * original file "c:/gte/properties/some.prop".
     *
     * Examples:
     *     guid("d:/GlobalTerrainEditor-release/data/properties/LoadMark.prop");
     *     guid("d:/GlobalTerrainEditor-release/data/Sounds/warning.wav.meta");
     *     guid("core/textures/water.tga");
     *
     * @returns empty if failed.
     *
     * Note that once @p relFileName was not found in gte_data and world dirs, it looks up in unigine
     * guid database(guid.db) in the gte and the world.
     * Which will cause a lack of perfomance in first call;
     */
    std::string guid(std::filesystem::path const& fileName) const;

    /**
     * Retrieves a relative file name to the GTE data or the world dir for specified @p guid.
     * @param guid unigine guid str(e.g. "9baa3e95ea88cd4a62620bb25d2a6b08ad8e9655").
     * @returns empty if failed.
     *
     * Note this is always cause to load unigine's guid db on first invoke.
     */
    std::filesystem::path fileName(std::string const& guid) const;

    /**
     * Gathers all guids in the specified @p relativeWorldDir relative to world dir.
     * e.g:
     * worldGuids("vector_data/debug") returns:
     * 
     * "cff548f3a6af79b6809ca59d5ab4ea58cf95e9d7" -> "vector_data/debug/Bridge_Marks_18.spl",
     * "9bdef6881043a84773d9c8fa84fbc41172ffae3c" -> "vector_data/debug/Bridge_Marks_06.spl"
     */
    std::unordered_map<std::string, std::filesystem::path> worldGuids(
        std::filesystem::path const& relativeWorldDir) const;

private:
    struct Record
    {
        std::string guid;
        std::filesystem::path fileName;
    };

    using Records = boost::multi_index::multi_index_container<
        Record,
        boost::multi_index::indexed_by<
            boost::multi_index::hashed_unique<
                boost::multi_index::tag<struct RecordByGuid>,
                boost::multi_index::member<Record, decltype(Record::guid), &Record::guid>
            >,
            boost::multi_index::hashed_unique<
                boost::multi_index::tag<struct RecordByFileName>,
                boost::multi_index::member<Record, decltype(Record::fileName), &Record::fileName>
            >
        >
    >;

    static void buildFrom(std::filesystem::path const& dbFileName, Records& records);
    static std::optional<std::string> extractGuidFromFile(std::filesystem::path metaFileName);
    static std::filesystem::path const& guidsFileName();
    static std::filesystem::path const& metaExtension();

    void maybeLoadDatabases() const;

    mutable std::optional<Records> m_database;

    const std::filesystem::path m_gteDataDir;
    const std::filesystem::path m_worldDir;
};
