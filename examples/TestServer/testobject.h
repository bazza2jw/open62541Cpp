#ifndef TESTOBJECT_H
#define TESTOBJECT_H
#include <open62541cpp/serverobjecttype.h>
#include "testmethod.h"
// Object Type Example
class TestObject : public Open62541::ServerObjectType
{
public:
    TestObject(Open62541::Server &s) : ServerObjectType(s,"TestObject")
    {

    }

    bool addChildren(const Open62541::NodeId &parent) override {
        Open62541::NodeId n;
        Open62541::NodeId a;
        return addObjectTypeVariable<double>("Current", parent, n.notNull()) &&
            addObjectTypeVariable<double>("Average", parent, a.notNull());
    }

};

#endif // TESTOBJECT_H
