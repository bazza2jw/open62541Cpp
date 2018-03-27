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

