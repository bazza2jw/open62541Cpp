Open62541 C++ Library for Open62541 version 0.3
===============================================

This is a set of wrapper classes for the Open62541 C OPC UA library version 0.3.

The objective is to reduce the code required by a considerable amount and allow object orrentated coding.

Building uses cmake

The examples show how to use the classes and should correspond to many of the C library examples.

Do not assume any OPC UA feature is implemented or complete or optimally done. Support will be added as and when it is needed. Feel free to constructively comment or contribute.

General Principles.
==================

Most UA_* types are wrapped such that for each UA_* structure there is a correspond C++ class to manage it.

Hence UA_NodeId is wrapped with Open62541::NodeId

Assignments or copy construction always uses deep copies.

It is possible to shallow copy UA_* items to the corresponding C++ managed objects. Do not do it, unless you really, really want to.

Context pointers attached to nodes on creation are assumed to be objects derived from Open62541::NodeContext.

The NodeContext class includes generalised handing of DataValue, node contstructor/destructor, and value callbacks.

There is C++ style class support of Object Types.
