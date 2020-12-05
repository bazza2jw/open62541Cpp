
set(LIB_HEADERS
        condition.h
        open62541objects.h
        open62541client.h
        open62541server.h
        propertytree.h
        clientcache.h
        clientcachethread.h
        nodecontext.h
        servermethod.h
        serverrepeatedcallback.h
        serverobjecttype.h
        serverbrowser.h
        servernodetree.h
        clientnodetree.h
        clientbrowser.h
        monitoreditem.h
        clientsubscription.h
        open62541cpp_trace.h
        discoveryserver.h
        historydatabase.h
        )

install(FILES ${LIB_HEADERS} DESTINATION include/open62541cpp)
