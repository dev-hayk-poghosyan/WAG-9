#include "ProtoMessageToJsonStream.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/type_resolver_util.h>

#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>

using namespace google::protobuf;
using namespace google::protobuf::io;
using namespace google::protobuf::util;

void protoMessageToJsonStream(google::protobuf::Message const& msg, std::ostream& strm)
{
    JsonPrintOptions opt;
    opt.add_whitespace = true;
    opt.always_print_fields_with_no_presence = true;

    protoMessageToJsonStream(msg, strm, opt);
}

void protoMessageToJsonStream(google::protobuf::Message const& msg, std::ostream& strm,
    google::protobuf::util::JsonPrintOptions option)
{
    std::stringstream binaryBuffer;
    if (!msg.SerializeToOstream(&binaryBuffer))
    {
        throw std::runtime_error("Failed to serialize protobuf message to binary form");
    }

    Descriptor const* descriptor = msg.GetDescriptor();

    DescriptorPool const* pool = descriptor->file()->pool();

    std::string const urlPrefix;
    std::unique_ptr<TypeResolver> resolver(NewTypeResolverForDescriptorPool(urlPrefix, pool));

    IstreamInputStream binaryInputStrm(&binaryBuffer);
    OstreamOutputStream jsonOutputStrm(&strm);
    
    absl::Status const status = BinaryToJsonStream(
        resolver.get(), urlPrefix + "/" + descriptor->full_name(),
        &binaryInputStrm, &jsonOutputStrm, option);

    if (!status.ok())
    {
        throw std::runtime_error(std::string(status.message()));
    }
}
