#ifndef MAP_ROUTING_DATA_H
#define MAP_ROUTING_DATA_H

enum
{
    CITIZEN_0_ROAD = 0,
    CITIZEN_2_PASSABLE_TERRAIN = 2,
    CITIZEN_4_CLEAR_TERRAIN = 4,
    CITIZEN_N1_BLOCKED = -1,
    CITIZEN_N3_AQUEDUCT = -3,
    CITIZEN_N4_RESERVOIR_CONNECTOR = -4,

    NONCITIZEN_0_PASSABLE = 0,
    NONCITIZEN_1_BUILDING = 1,
    NONCITIZEN_2_CLEARABLE = 2,
    NONCITIZEN_3_WALL = 3,
    NONCITIZEN_4_GATEHOUSE = 4,
    NONCITIZEN_5_FORT = 5,
    NONCITIZEN_N1_BLOCKED = -1,

    WATER_0_PASSABLE = 0,
    WATER_N1_BLOCKED = -1,
    WATER_N2_MAP_EDGE = -2,
    WATER_N3_LOW_BRIDGE = -3,

    WALL_0_PASSABLE = 0,
    WALL_N1_BLOCKED = -1,
};

#endif // MAP_ROUTING_DATA_H
