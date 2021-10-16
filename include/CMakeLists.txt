
file(GLOB LIB_HEADERS
        "open62541cpp/*.h"
)

install(FILES ${LIB_HEADERS} DESTINATION include/open62541cpp)
