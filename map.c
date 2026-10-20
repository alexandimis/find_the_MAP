// TODO (-), DONE (X)
// - Add Types in rooms
// X Handle multy-connected rooms as one big with cap: MAX_COMBINED_ROOMS


#include "map.h"
#include "random.h"
#include "safeio.h"

extern uint16_t seed;
Map_t *map; // Do not make static. Other files might want access to it.


// ######################################
// ### Possition/ID encoding/decoding ###
// ######################################

// Convert signed to unsigned
static uint16_t encode_signed(int val){
    return (uint16_t)(val + 256) & UINT32_C(0xFF);
}

// Convert unsigned to signed
static int decode_signed(uint16_t val){
    val &= UINT16_C(0xFF);
    // Sign check
    if (val & UINT16_C(0x80)){
        return (int)val - 256;
    }

    return (int)val;
}

// Generates a key for given position
uint16_t get_key(vec2 pos){
    return (encode_signed(pos.x) << 8 |
            encode_signed(pos.y));
}

// Extracts the position of a given key
static vec2 get_pos(uint16_t key){
    return (vec2){
        .x = decode_signed(key>>8),
        .y = decode_signed(key)
    };
}

// ########################
// ### Helper functions ###
// ########################

// Returns the component parent for Union-Find of a cell
Cell_t *component_find(Cell_t *cell){
    if(cell->component_parent != cell){
        cell->component_parent = component_find(cell->component_parent);
    }
    return cell->component_parent;
}

// Returns true if pos is within the map bounds
// Otherwise false
static bool in_bounds(const vec2 pos){
    return  XMIN < pos.x && pos.x < XMAX &&
            YMIN < pos.y && pos.y < YMAX;
}

// Returns the possition starting from cell 
// and moving by one step to the given dir
vec2 adjecent_pos(const Cell_t *cell, const Drctn dir){
    return (vec2){
        .x = cell->position.x + DirOffset[dir].x,
        .y = cell->position.y + DirOffset[dir].y
    };
}

// Returns the sum of two vec2
static vec2 vec2_add(vec2 a, vec2 b){
    return (vec2){
        .x = a.x + b.x,
        .y = a.y + b.y
    };
}

// Returns cell if found. Null otherwise
Cell_t *cell_find(const uint16_t key){
    Cell_t *cell = NULL;
    HASH_FIND(hh, map->cells, &key, sizeof(key), cell);
    return cell;
}

// ######################
// ### Applying Rules ###
// ######################

// Checks whether two cells can link.
// Applies Hallway/Room rules
static bool can_link(Cell_t *from, Cell_t *to, Drctn dir){
    if(!from || !to || from == to) return false;
    if(from->neighbor[dir] ||
        to->neighbor[OPOSITE_DIR[dir]]) return false;

    // Different types should be able to link normaly
    if(from->type != to->type) return true;

    Cell_t *root_a = component_find(from);
    Cell_t *root_b = component_find(to);

    switch (from->type)
    {
    case HALLWAY:
        return (root_a != root_b);
    case ROOM:
        // Linking rooms already in the same component 
        // does not increase its size
        if(root_a == root_b) return true;

        // Only link if the total size does not exceed the limit
        return (root_a->component_size + root_b->component_size <= MAX_COMBINED_ROOMS);
    default:
        break;
    }

    return true;
}

// Applies generation rules and returns true if a cell can be created/linked in that direction
static bool can_expand(Cell_t *cell, Drctn dir){
    if(!cell) return false;
    if(cell->neighbor[dir]) return false;

    vec2 pos = adjecent_pos(cell, dir);
    if(!in_bounds(pos)) return false;

    Cell_t *to = cell_find(get_key(pos));

    // Empty cell.
    if(!to) return true;

    return can_link(cell, to, dir);
}

// Returns a random neighbor direction from which the generation can continue
static Drctn random_extendable_dir(Cell_t *cell){
    // Tetris-like idea.
    // Generate a box with all of your options first, then pick one at random
    Drctn box[4] = {-1,-1,-1,-1};
    uint8_t count = 0;
    for(Drctn dir = 0; dir < 4; dir++){
        if(!can_expand(cell, dir)) continue;

        box[count++] = dir;
    }

    if(count == 0) return (Drctn)-1;

    return box[rng_bounded(count)];
}

// Returns a random type that a cell can take
// next to 'from' at direction 'dir'
// If there are no options a random special type is selected instead
static CellType_t random_valid_type(Cell_t *from, Drctn dir){
    if(!from) return SPECIAL1; // Find a better rule for this case later
    // if(!from) return EMPTY; // Find a better rule for this case later
    if(from->neighbor[dir]) return from->type;

    CellType_t box[TYPE_COUNT];
    uint8_t count = 0;
    for(CellType_t type = 0; type < TYPE_COUNT; type++){
        // If the current component is ROOM with max size you can't add more
        // So skip type == ROOM
        // Make sure to check the component size of the parent, not the cell
        if(from->type == ROOM && 
            type == ROOM &&
            from->component_parent->component_size >= MAX_COMBINED_ROOMS) continue;
        
        box[type] = type;
        count++;
    }

    // Add special cell types later
    if(count == 0) return SPECIAL1;

    return box[rng_bounded(count)];
}


static void component_union(Cell_t *from, Cell_t *to){
    // A union consists only of the same type
    if(from->type != to->type) return;

    Cell_t *root_a = component_find(from);
    Cell_t *root_b = component_find(to);

    // If they are already united return
    if(root_a == root_b) return;

    // Union by size: attach smaller to larger
    if(root_a->component_size < root_b->component_size){
        // Swap
        Cell_t *tmp = root_a;
        root_a = root_b;
        root_b = tmp;
    }

    root_b->component_parent = root_a;
    root_a->component_size += root_b->component_size;

}

// Links/Connects two cells. Provided direction is 'from'->'to'
// Returns false for failing. True for linking
static bool cell_link(Cell_t *from, Cell_t *to, Drctn dir){
    if(!can_link(from, to, dir)) return false;

    int opositeDir = OPOSITE_DIR[dir];

    // Link them
    from->neighbor[dir] = to;
    to->neighbor[opositeDir] = from;

    
    // Update state if hallway
    if(from->type == HALLWAY){
        from->hallway.facing |= dir_bitmask[dir];
    }
    if(to->type == HALLWAY){
        to->hallway.facing |= dir_bitmask[opositeDir];
    }
    if(from->type == to->type){
        component_union(from, to);
    }

    return true;
}

// ####################
// ### Actual logic ###
// ####################

// Creates a cell. 'type' & 'dir' are optional arguments, provided for specific cases
Cell_t *cell_create(Cell_t *from, Drctn dir){
// Cell_t *cell_create(uint16_t key, CellType_t type, Drctn dir){

    // All generated rooms should be connected
    if(!from || dir == -1) return NULL;

    vec2 pos = vec2_add(from->position, DirOffset[dir]);    // Possition of the new cell
    uint16_t key = get_key(pos);

    Cell_t *cell = cell_find(key);
    if(cell) return cell;

    // Use 'type' if it's provided. Otherwise get a random type
    CellType_t type = random_valid_type(from, dir);

    cell = xcalloc(1, sizeof(Cell_t));


    for(Drctn dir = 0; dir < 4; dir++){
        cell->neighbor[dir] = NULL;
    }
    
    cell->type = type;

    cell->component_parent = cell;
    cell->component_size = 1;
    
    cell->position = get_pos(key);
    cell->key = key;
    cell->id = map->cellc++;
    HASH_ADD(hh, map->cells, key, sizeof(key), cell);
    return cell;
}

// Map is stored in hash map
static void map_init(void){
    map = (Map_t*)xcalloc(1, sizeof(Map_t));
    map->cellc = 0;
    map->totLinks = 0;
    map->cells = NULL;
}

uint8_t available_neighbors(Cell_t *cell){
    uint8_t count = 0;
    for(Drctn dir = 0; dir < 4; dir++){
        count += can_expand(cell, dir);
    }
    return count;
}

void map_generate_recursive(Cell_t *cell){
    uint8_t available = available_neighbors(cell);
    if(available == 0) return;

    uint8_t branches = 1 + rng_bounded(available);    // Range of [0,available]

    for(uint8_t i = 0; i < branches; i++){

        Drctn dir = random_extendable_dir(cell);
        if(dir == -1) break;  // No available dir

        // Calculate new position
        vec2 pos = adjecent_pos(cell, dir);
        uint16_t key = get_key(pos);

        if(!in_bounds(pos)) continue;

        Cell_t *to = cell_find(key);

        bool created = (to == NULL);

        if(created){
            to = cell_create(cell, dir);
        }

        if(!cell_link(cell, to, dir)) continue;

        // Continue recursively only if a cell was created
        if(created){
            map_generate_recursive(to);
        }
    }
}

void map_generate(const uint8_t seed){
    map_init();
    rng_init(seed);


    // The spawn is the only cell which will be generated
    // without 'create_cell' because it starts isolated
    Cell_t *spawn = xcalloc(1, sizeof(Cell_t));

    uint16_t key = get_key((vec2){0,0});

    for(Drctn dir = 0; dir < 4; dir++){
        spawn->neighbor[dir] = NULL;
    }
    
    spawn->type = ROOM;
    // spawn->type = EMPTY;

    spawn->component_parent = spawn;
    spawn->component_size = 1;
    
    spawn->position = get_pos(key);
    spawn->key = key;
    spawn->id = map->cellc++;
    HASH_ADD(hh, map->cells, key, sizeof(key), spawn);

    map_generate_recursive(spawn);
}

void map_destroy(void){
    Cell_t *cur = NULL;
    Cell_t *tmp = NULL;

    // Free hash table
    HASH_ITER(hh, map->cells, cur, tmp) {
        HASH_DEL(map->cells, cur);
        free(cur);
    }

    free(map);
}