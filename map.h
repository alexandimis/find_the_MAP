#ifndef MAP_H
#define MAP_H

#include<stdint.h>
#include<stdbool.h>

#include "deps/uthash.h"

#define MAX_COMBINED_ROOMS 3
#define XMIN -5
#define XMAX 5
#define YMIN -5
#define YMAX 5

// Bit mask of directions
typedef enum{
    DIR_EAST    = 0,    // +x
    DIR_WEST    = 1,    // -x
    DIR_NORTH   = 2,    // +y
    DIR_SOUTH   = 3,    // -y
} Drctn;

static const char dir_bitmask[4] = {
    [DIR_EAST]  = 1,
    [DIR_WEST]  = 2,
    [DIR_NORTH] = 4,
    [DIR_SOUTH] = 8
};

typedef enum{
    // The compiler will set the values of the following types to increase by 1.
    // This way negative values will never appear
    // And 'TYPE_COUNT' results directly to the number of elements that are inside the enum.
    // This includes 'TYPE_COUNT' so it will frequently be used as TYPE_COUNT-1

    // EMPTY = 0,  // Null?
    HALLWAY = 0,
    ROOM,

    TYPE_COUNT,

    // Special types are placed about 'TYPE_COUNT' 
    // so that they are not included by 'rng_bounds(TYPE_COUNT)'
    // Another counter can be added later if we need to randomize specials
    SPECIAL1
} CellType_t;


// typedef enum{
// } Rarity_t;

typedef struct {
    uint8_t id;

    union{
        struct{
            // Rarity_t rarity;
        }map;

        struct{

        }shovel;

        struct{

        }bucket;
    };
    
} Items_t;

typedef struct{
    int x,y;
} vec2;

typedef struct Cell{
    CellType_t type;

    // Used to apply cell rules by using Union-Find
    struct Cell *component_parent;
    uint16_t component_size;
    
    union{
        // struct empty{
        // };

        struct {
            Drctn facing;   // Used as a bitmask for visuals

            // Used to avoid single hallway loops using Union-Find
            // struct Cell *hallway_parent;
        }hallway;

        struct {
            uint16_t size;
            // Used to avoid more than 5 connected rooms using Union-Find
            // struct Cell *room_parent;
        }room;
    };
    

    struct Cell *neighbor[4];   // 4 dirrections: NORTH,SOUTH,EAST,WEST
    vec2 position;
    uint8_t id;                 // Linear, Increasing by 1
    Items_t item;

    // Hash handle
    UT_hash_handle hh;
    uint32_t key;               // 32 bit encoded position used as the hash table key
} Cell_t;

typedef struct{
    uint8_t cellc;
    uint16_t totLinks;

    // UTHASH head ptr
    Cell_t *cells;
} Map_t;


static const vec2 DirOffset[4] = {
    [DIR_EAST] = (vec2){ 1, 0},
    [DIR_WEST] = (vec2){-1, 0},
    [DIR_NORTH]= (vec2){ 0, 1},
    [DIR_SOUTH]= (vec2){ 0,-1}
};

static const Drctn OPOSITE_DIR[4] = {
    [DIR_EAST] = DIR_WEST,
    [DIR_WEST] = DIR_EAST,
    [DIR_NORTH]= DIR_SOUTH,
    [DIR_SOUTH]= DIR_NORTH
};

void map_generate(const uint8_t seed);
void map_destroy(void);


Cell_t *cell_find(uint16_t key);
vec2 adjecent_pos(const Cell_t *cell, const Drctn dir);
uint16_t get_key(vec2 pos);
Cell_t *component_find(Cell_t *cell);


#endif