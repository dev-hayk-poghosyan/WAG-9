#pragma once

#include "LayerXmlParseStatusCode.h"

#include <exception>
#include <string>
#include <vector>

/**
 * Contains the code and parameters of the situation that occurred during the parsing a
 * layer XML file. 
 */
struct LayerXmlParseStatus
{
    LayerXmlParseStatus(const std::vector<LayerXmlParseStatus>& aSubstatuses,
        std::string const& aFilePath, std::string const& aNodeName, std::string const& anErrorMsg,
        LayerXmlParseStatusCode aStatusCode)
        :filePath(aFilePath)
        , nodeName(aNodeName)
        , errorMsg(anErrorMsg)
        , statusCode(aStatusCode)
    {
    }

    LayerXmlParseStatus(std::string const& aFilePath, LayerXmlParseStatusCode aStatusCode)
        : filePath(aFilePath)
        , nodeName({})
        , errorMsg({})
        , statusCode(aStatusCode)
    {
    }

    LayerXmlParseStatus(LayerXmlParseStatusCode aStatusCode)
        : filePath({})
        , nodeName({})
        , errorMsg({})
        , statusCode(aStatusCode)
    {
    }

    LayerXmlParseStatus(std::string const& aFilePath, std::string const& aNodeName,
        LayerXmlParseStatusCode aStatusCode)
        : filePath(aFilePath)
        , nodeName(aNodeName)
        , errorMsg({})
        , statusCode(aStatusCode)
    {
    }

    LayerXmlParseStatus(std::string const& aFilePath, std::string const& aNodeName,
        std::string const& anErrorMsg, LayerXmlParseStatusCode aStatusCode)
        : filePath(aFilePath)
        , nodeName(aNodeName)
        , errorMsg(anErrorMsg)
        , statusCode(aStatusCode)
    {
    }

    LayerXmlParseStatus(const LayerXmlParseStatus& other)
        : filePath(other.filePath)
        , nodeName(other.nodeName)
        , errorMsg(other.errorMsg)
        , statusCode(other.statusCode)
    {
    }

    const std::string filePath;
    const std::string nodeName;
    const std::string errorMsg;
    const LayerXmlParseStatusCode statusCode;
};

struct LayerXmlParseException : public std::exception
{
    LayerXmlParseException(const LayerXmlParseStatus& aMessage)
        : std::exception()
        , message(aMessage)
    {
    }
    LayerXmlParseStatus message;
};
