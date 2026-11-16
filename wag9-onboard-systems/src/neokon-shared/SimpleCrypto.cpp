#include "SimpleCrypto.h"

#include <boost/format.hpp>
#include <boost/uuid/detail/sha1.hpp>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/type_resolver_util.h>

#include <fstream>
#include <stdexcept>
#include <vector>

std::vector<uint32_t>
seedGen(std::vector<uint32_t> in)
{
    std::vector<uint32_t> res = in;
    volatile int seed = 1;

    boost::uuids::detail::sha1 sha1;
    for (auto& e : res)
    {
        e += seed;
        seed += 3;
    }
    return res;
}

void
simpleEncryptToFile(void* data, size_t size, std::ofstream& fileStream)
{
    // Seed gen

    //
    auto seed = seedGen(std::vector<uint32_t>(4, 4));

    constexpr int shaBytesPerRequest = 5 * sizeof(unsigned int);

    std::vector<uint8_t> straemBytes;
    straemBytes.resize(shaBytesPerRequest);
    int streamReadPos = straemBytes.size();

    boost::uuids::detail::sha1 sha1;

    for (size_t i = 0; i < size; i++)
    {
        if (streamReadPos >= straemBytes.size())
        {
            sha1.process_bytes(seed.data(), seed.size() * sizeof(*seed.data()));

            static_assert(
                sizeof(boost::uuids::detail::sha1::digest_type) == shaBytesPerRequest, "Sha1 digest not 20? Api changed?");
            boost::uuids::detail::sha1::digest_type digest;
            sha1.get_digest(digest);

            std::memcpy(straemBytes.data(), digest, shaBytesPerRequest);

            seed[0]++;
            streamReadPos = 0;
        }

        fileStream << uint8_t(((uint8_t*)data)[i] ^ straemBytes[streamReadPos]);

        streamReadPos++;
    }
}

std::string
simpleDecryptFromFile(std::ifstream& fileStream)
{
    std::string data = {
        std::istreambuf_iterator<char>(fileStream), std::istreambuf_iterator<char>()};
    // Seed gen

    size_t size = data.size();

    //
    auto seed = seedGen(std::vector<uint32_t>(4, 4));

    const int shaBytesPerRequest = 5 * sizeof(unsigned int);

    std::vector<uint8_t> straemBytes;
    straemBytes.resize(shaBytesPerRequest);
    int streamReadPos = straemBytes.size();

    boost::uuids::detail::sha1 sha1;

    for (size_t i = 0; i < size; i++)
    {
        if (streamReadPos >= straemBytes.size())
        {
            sha1.process_bytes(seed.data(), seed.size() * sizeof(*seed.data()));

            static_assert(
                sizeof(boost::uuids::detail::sha1::digest_type) == shaBytesPerRequest,
                "Sha1 digest not 20? Api changed?");
            boost::uuids::detail::sha1::digest_type digest;
            sha1.get_digest(digest);

            std::memcpy(straemBytes.data(), digest, shaBytesPerRequest);

            seed[0]++;
            streamReadPos = 0;
        }

        data[i] = uint8_t(data[i] ^ straemBytes[streamReadPos]);

        streamReadPos++;
    }

    return data;
}

using namespace google::protobuf;
using namespace google::protobuf::io;
using namespace google::protobuf::util;

void
simpleEncryptProto(google::protobuf::Message& msg, std::filesystem::path const& filePath)
{
    JsonPrintOptions opt;
    opt.add_whitespace = true;
    opt.always_print_fields_with_no_presence = true;

    std::string binaryString;
    if (!msg.SerializeToString(&binaryString))
    {
        throw std::runtime_error("Failed to serialize protobuf message to binary form");
    }

    Descriptor const* descriptor = msg.GetDescriptor();

    DescriptorPool const* pool = descriptor->file()->pool();

    std::string const urlPrefix;
    std::unique_ptr<TypeResolver> resolver(NewTypeResolverForDescriptorPool(urlPrefix, pool));

    std::string strm;

    std::string fullJsonString;

    absl::Status const status = BinaryToJsonString(
        resolver.get(), urlPrefix + "/" + descriptor->full_name(), binaryString , &fullJsonString,
        opt);

    if (!status.ok())
    {
        throw std::runtime_error(std::string(status.message()));
    }

    std::ofstream f(filePath, std::ios::binary);

    simpleEncryptToFile(
        fullJsonString.data(), fullJsonString.size() * sizeof(*fullJsonString.data()), f);
}

void
simpleDecryptProto(std::filesystem::path const& filePath, google::protobuf::Message& msg)
{
    std::ifstream fileStream(filePath, std::ios::binary);
    auto dataEncrypted = simpleDecryptFromFile(fileStream);

    Descriptor const* descriptor = msg.GetDescriptor();

    DescriptorPool const* pool = descriptor->file()->pool();

    std::string const urlPrefix;
    std::unique_ptr<TypeResolver> resolver(NewTypeResolverForDescriptorPool(urlPrefix, pool));

    JsonParseOptions opt;
    opt.ignore_unknown_fields = true;

    absl::Status const status = JsonStringToMessage(dataEncrypted, &msg);

    if (!status.ok())
    {
        throw std::runtime_error(std::string(status.message()));
    }
}

