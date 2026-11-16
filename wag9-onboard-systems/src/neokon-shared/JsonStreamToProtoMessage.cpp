#include "JsonStreamToProtoMessage.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/type_resolver_util.h>

#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>

using namespace google::protobuf;
using namespace google::protobuf::io;
using namespace google::protobuf::util;

void jsonStreamToProtoMessage(std::istream& strm, google::protobuf::Message& msg)
{
    Descriptor const* descriptor = msg.GetDescriptor();

    DescriptorPool const* pool = descriptor->file()->pool();

    std::string const urlPrefix;
    std::unique_ptr<TypeResolver> resolver(NewTypeResolverForDescriptorPool(urlPrefix, pool));

    std::stringstream binaryBuffer;

    // Apparently the OstreamOutputStream below does some additional writing from its
    // destructor, which is why it has to be destroyed before calling msg.ParseFromIstream() below.
    {
        IstreamInputStream jsonInputStrm(&strm);
        OstreamOutputStream binaryOutputStrm(&binaryBuffer);

        JsonParseOptions opt;
        opt.ignore_unknown_fields = true;

        absl::Status const status = JsonToBinaryStream(
            resolver.get(), urlPrefix + "/" + descriptor->full_name(),
            &jsonInputStrm, &binaryOutputStrm, opt);

        if (!status.ok())
        {
            throw std::runtime_error(std::string(status.message()));
        }

    }

    if (!msg.ParseFromIstream(&binaryBuffer))
    {
        throw std::runtime_error("Failed to load a protobuf message from binary form");
    }
}
