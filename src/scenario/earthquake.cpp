#include "earthquake.h"

#include "city/message.h"
#include "core/calc.h"
#include "core/random.h"
#include "scenario/data.h"
#include "sound/effect.h"

#include <data>
#include <game>

#include "scenario/building.h"

#include "figure.h"

#include "terraingraphics.h"

void Building_collapseOnFire(int buildingId, int hasPlague);
void Building_collapseLinked(int buildingId, int onFire);

enum
{
    EARTHQUAKE_NONE = 0,
    EARTHQUAKE_SMALL = 1,
    EARTHQUAKE_MEDIUM = 2,
    EARTHQUAKE_LARGE = 3
};

static struct
{
    int game_year;
    int month;
    int state;
    int duration;
    int max_duration;
    int delay;
    int max_delay;
    struct
    {
        int x;
        int y;
    } expand[4];
} data;

void scenario_earthquake_init()
{
    data.game_year = scenario.start_year + scenario.earthquake.year;
    data.month = 2 + (random_byte() & 7);
    switch (scenario.earthquake.severity)
    {
    default:
        data.max_duration = 0;
        data.max_delay = 0;
        break;
    case EARTHQUAKE_SMALL:
        data.max_duration = 25 + (random_byte() & 0x1f);
        data.max_delay = 10;
        break;
    case EARTHQUAKE_MEDIUM:
        data.max_duration = 100 + (random_byte() & 0x3f);
        data.max_delay = 8;
        break;
    case EARTHQUAKE_LARGE:
        data.max_duration = 250 + random_byte();
        data.max_delay = 6;
        break;
    }
    data.state = EVENT_NOT_STARTED;
    for (int i = 0; i < 4; i++)
    {
        data.expand[i].x = scenario.earthquake_point.x;
        data.expand[i].y = scenario.earthquake_point.y;
    }
}

static int canAdvanceEarthquakeToTile(int x, int y)
{
    int terrain = Data_Grid_terrain[GridOffset(x, y)];
    if (terrain & (Terrain_Elevation | Terrain_Rock | Terrain_Water))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

static void advanceEarthquakeToTile(int x, int y)
{
    int gridOffset = GridOffset(x, y);
    int buildingId = Data_Grid_buildingIds[gridOffset];
    if (buildingId)
    {
        Building_collapseOnFire(buildingId, 0);
        Building_collapseLinked(buildingId, 1);
        sound_effect_play(SOUND_EFFECT_EXPLOSION);
        Data_Buildings[buildingId].state = BuildingState_DeletedByGame;
    }
    Data_Grid_terrain[gridOffset] = 0;
    TerrainGraphics_setTileEarthquake(x, y);
    TerrainGraphics_updateAllGardens();
    TerrainGraphics_updateAllRoads();
    TerrainGraphics_updateRegionPlazas(0, 0, Data_State.map.width - 1, Data_State.map.height - 1);

    map_routing_update_land();
    map_routing_update_walls();

    Figure_createDustCloud(x, y, 1);
}

void scenario_earthquake_process()
{
    if (scenario.earthquake.severity == EARTHQUAKE_NONE ||
            scenario.earthquake_point.x == -1 || scenario.earthquake_point.y == -1)
    {
        return;
    }
    if (data.state == EVENT_NOT_STARTED)
    {
        if (game_time_year() == data.game_year &&
                game_time_month() == data.month)
        {
            data.state = EVENT_IN_PROGRESS;
            data.duration = 0;
            data.delay = 0;
            advanceEarthquakeToTile(data.expand[0].x, data.expand[0].y);
            city_message_post(1, MESSAGE_EARTHQUAKE, 0,
                              GridOffset(data.expand[0].x, data.expand[0].y));
        }
    }
    else if (data.state == EVENT_IN_PROGRESS)
    {
        data.delay++;
        if (data.delay >= data.max_delay)
        {
            data.delay = 0;
            data.duration++;
            if (data.duration >= data.max_duration)
            {
                data.state = EVENT_FINISHED;
            }
            int dx, dy, index;
            switch (random_byte() & 0xf)
            {
            case 0:
                index = 0;
                dx = 0;
                dy = -1;
                break;
            case 1:
                index = 1;
                dx = 1;
                dy = 0;
                break;
            case 2:
                index = 2;
                dx = 0;
                dy = 1;
                break;
            case 3:
                index = 3;
                dx = -1;
                dy = 0;
                break;
            case 4:
                index = 0;
                dx = 0;
                dy = -1;
                break;
            case 5:
                index = 0;
                dx = -1;
                dy = 0;
                break;
            case 6:
                index = 0;
                dx = 1;
                dy = 0;
                break;
            case 7:
                index = 1;
                dx = 1;
                dy = 0;
                break;
            case 8:
                index = 1;
                dx = 0;
                dy = -1;
                break;
            case 9:
                index = 1;
                dx = 0;
                dy = 1;
                break;
            case 10:
                index = 2;
                dx = 0;
                dy = 1;
                break;
            case 11:
                index = 2;
                dx = -1;
                dy = 0;
                break;
            case 12:
                index = 2;
                dx = 1;
                dy = 0;
                break;
            case 13:
                index = 3;
                dx = -1;
                dy = 0;
                break;
            case 14:
                index = 3;
                dx = 0;
                dy = -1;
                break;
            case 15:
                index = 3;
                dx = 0;
                dy = 1;
                break;
            default:
                return;
            }
            int x = calc_bound(data.expand[index].x + dx, 0, Data_State.map.width - 1);
            int y = calc_bound(data.expand[index].y + dy, 0, Data_State.map.height - 1);
            if (canAdvanceEarthquakeToTile(x, y))
            {
                data.expand[index].x = x;
                data.expand[index].y = y;
                advanceEarthquakeToTile(x, y);
            }
        }
    }
}

int scenario_earthquake_is_in_progress()
{
    return data.state == EVENT_IN_PROGRESS;
}

void scenario_earthquake_save_state(buffer *buf)
{
    buffer_write_i32(buf, data.game_year);
    buffer_write_i32(buf, data.month);
    buffer_write_i32(buf, data.state);
    buffer_write_i32(buf, data.duration);
    buffer_write_i32(buf, data.max_duration);
    buffer_write_i32(buf, data.max_delay);
    buffer_write_i32(buf, data.delay);
    for (int i = 0; i < 4; i++)
    {
        buffer_write_i32(buf, data.expand[i].x);
        buffer_write_i32(buf, data.expand[i].y);
    }
}

void scenario_earthquake_load_state(buffer *buf)
{
    data.game_year = buffer_read_i32(buf);
    data.month = buffer_read_i32(buf);
    data.state = buffer_read_i32(buf);
    data.duration = buffer_read_i32(buf);
    data.max_duration = buffer_read_i32(buf);
    data.max_delay = buffer_read_i32(buf);
    data.delay = buffer_read_i32(buf);
    for (int i = 0; i < 4; i++)
    {
        data.expand[i].x = buffer_read_i32(buf);
        data.expand[i].y = buffer_read_i32(buf);
    }
}
