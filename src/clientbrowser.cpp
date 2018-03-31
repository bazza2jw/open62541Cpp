/*
 * Copyright (C) 2017 -  B. J. Hill
 *
 * This file is part of open62541 C++ classes. open62541 C++ classes are free software: you can
 * redistribute it and/or modify it under the terms of the Mozilla Public
 * License v2.0 as stated in the LICENSE file provided with open62541.
 *
 * open62541 C++ classes are distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.
 */
#include <clientbrowser.h>
/*!
    \brief Open62541::ClientBrowser::browseIter
    \param childId
    \param isInverse
    \param referenceTypeId
    \param handle
    \return
*/
UA_StatusCode Open62541::ClientBrowser::browseIter(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle) {
    // node iterator for browsing
    if (isInverse) return UA_STATUSCODE_GOOD; // TO DO what does this do?
    Open62541::ClientBrowser *p = (Open62541::ClientBrowser *)handle;
    if (p) {
        p->process(childId, referenceTypeId); // process record
    }
    return UA_STATUSCODE_GOOD;
}

