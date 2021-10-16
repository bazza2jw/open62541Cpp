#ifndef TESTOBJECT_H
#define TESTOBJECT_H
#include <open62541cpp/serverobjecttype.h>
#include <open62541cpp/propertytree.h>
#include <open62541cpp/open62541objects.h>
#include "testmethod.h"
// Object Type Example
class TestObject : public Open62541::ServerObjectType
{
public:
    TestObject(Open62541::Server& s)
        : ServerObjectType(s, "AR_ObjectType")
    {
    }
    bool addChildren(const Open62541::NodeId& parent) override
    {
        Open62541::NodeId n;
        Open62541::NodeId a;
        Open62541::NodeId b;
        return addObjectTypeVariable<double>("Current", parent, n.notNull()) &&
               addDerivedObjectType ("Golash", parent, a.notNull())&&
               addObjectTypeArrayVariable<int, 5>("Average", a, b.notNull());
    }
};

#endif  // TESTOBJECT_H
