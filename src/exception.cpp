//
// Created by Stefan Profanter on 10/27/21.
// Copyright (c) 2021 Agile Robots AG. All rights reserved.
//

#include <open62541cpp/exception.h>

#include <open62541/types.h>

using namespace Open62541;

StringException::StringException(::std::string what_arg)
    : message(std::move(what_arg))
{
}

StringException::StringException(const char* what_arg)
    : message(what_arg)
{
}

StringException::StringException(const StringException& other)
    : message(other.message)
{
}

auto StringException::operator=(const StringException& other) -> StringException&
{
    // check for self-assignment
    if (&other == this) {
        return *this;
    }
    message = other.message;
    return *this;
}

StringException::StringException(StringException&& other) noexcept
    : message(std::move(other.message))
{
}

auto StringException::operator=(StringException&& other) noexcept -> StringException&
{
    // check for self-assignment
    if (&other == this) {
        return *this;
    }
    message = std::move(other.message);
    return *this;
}

auto StringException::what() const noexcept -> const char*
{
    return message.c_str();
}

StringException::~StringException() noexcept = default;

StatusCodeException::StatusCodeException(UA_StatusCode code, const std::string& msg)
    : StringException("StatusCode: " + std::string(UA_StatusCode_name(code)) + (msg.length() > 0 ? ". " + msg : ""))
    , code_(code)
{
}