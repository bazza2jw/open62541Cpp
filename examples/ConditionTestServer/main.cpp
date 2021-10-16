#include <iostream>
//
// this is the C++ version of the tutorial_server_alarms_conditions.c example
// this exercises the server condition code
//
#include "conditiontestserver.h"

using namespace std;
//
int main()
{
    ConditionServer server;
    server.start();
    return 0;
}
