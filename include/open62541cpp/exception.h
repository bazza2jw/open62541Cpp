//
// Created by Stefan Profanter on 10/27/21.
// Copyright (c) 2021 Agile Robots AG. All rights reserved.
//

#ifndef OPEN62541CPP_EXCEPTION_H
#define OPEN62541CPP_EXCEPTION_H

#include <exception>
#include <string>
#include <open62541/types.h>

namespace Open62541 {

/**
 * Base exception for all ar::dds::rpc exceptions
 */
class UA_EXPORT Exception : public ::std::exception
{
};

/**
 * Exception with sting message which can be thrown by other classes related to Links and Nodes.
 */
class UA_EXPORT StringException : public Exception
{
private:
    std::string message;

public:
    /**
     * Constructor creating an exception with given string message.
     * @param what_arg Description of the exception.
     */
    explicit StringException(::std::string what_arg);

    /**
     * Constructor creating an exception with given char message.
     * @param what_arg Description of the exception.
     */
    explicit StringException(const char* what_arg);

    ~StringException() noexcept override;

    /**
     * Copy constructor copying the message into this instance.
     * @param other Exception from where to copy the message.
     */
    [[maybe_unused]] StringException(const StringException& other);

    /**
     * Copy assignment constructor copying the message into this instance.
     * @param other Exception from where to copy the message.
     * @return an instance of an Exception with the copied content of the passed-in exception.
     */
    auto operator=(const StringException& other) -> StringException&;

    /**
     * Move constructor moving the content from given rvalue exception into this instance.
     * The exception given as parameter is not valid afterwards.
     * @param other Exception from which to take the value.
     */
    [[maybe_unused]] StringException(StringException&& other) noexcept;

    /**
     * Move assignment constructor moving the content from given rvalue exception into this instance.
     * The exception given as parameter is not valid afterwards.
     * @param other Exception from which to take the value.
     * @return An instance with the new exception content.
     */
    auto operator=(StringException&& other) noexcept -> StringException&;

    /**
     * Get the exception message stored in the exception.
     *
     * @return the message describing the exception.
     */
    [[nodiscard]] auto what() const noexcept -> const char* override;
};

/**
 * Exception class which can be used to throw an OPC UA Status code exception.
 */
class UA_EXPORT StatusCodeException : public StringException
{
private:
    UA_StatusCode code_;

public:
    /**
     * Constructor for a StatusCodeException.
     * The message will start with `StatusCode: <STATUS_CODE_NAME>` and the optional msg is added to that message.
     *
     * @param code The status code which is stored in the exception.
     * @param msg Additional message which is added to the what string.
     */
    explicit StatusCodeException(UA_StatusCode code, const std::string& msg = "");

    /**
     * Get the stored status code.
     * @return The status code used to initialize the exception.
     */
    [[nodiscard]] auto code() const -> UA_StatusCode { return code_; }
};

inline void throw_bad_status(const UA_StatusCode& code)
{
    if (code == UA_STATUSCODE_GOOD) {
        return;
    }
    throw StatusCodeException(code);
}

}  // namespace Open62541

#endif  // OPEN62541CPP_EXCEPTION_H
