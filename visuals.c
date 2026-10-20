#define _XOPEN_SOURCE_EXTENDED 1

#include <ncurses.h>
#include <locale.h>
#include "map.h"
#include "visuals.h"

extern Map_t *map;

static int map_to_screen_x(
    int room_x,
    int player_x,
    int screen_center_x)
{
    return screen_center_x + (room_x - player_x);
    // return screen_center_x + (room_x - player_x) * 2;
}

static int map_to_screen_y(
    int room_y,
    int player_y,
    int screen_center_y)
{
    return screen_center_y - (room_y - player_y);
    // return screen_center_y - (room_y - player_y) * 2;
}

static const cchar_t get_symbol(const Cell_t *player, const Cell_t *room){

    cchar_t symbol;

    if(player == room){
        setcchar(&symbol, L"@", A_NORMAL, 1, NULL);  // 1 -> Red foreground, Default background
        return symbol;
    }

    switch (room->type)
    {
    case HALLWAY:
        Drctn dir = room->hallway.facing;
        setcchar(&symbol, hallway_chars[dir], A_NORMAL, 0, NULL);   // 0 -> default
        break;
    case ROOM:
        setcchar(&symbol, L"0", A_NORMAL, 0, NULL);
        break;
    // case EMPTY:
        setcchar(&symbol, L"O", A_NORMAL, 0, NULL);
        break;
    case SPECIAL1:
        setcchar(&symbol, L"X", A_NORMAL, 0, NULL);
        break;
    default:
        setcchar(&symbol, L"E", A_NORMAL, 0, NULL);   // For error, should not come here
        break;
    }

    return symbol;
}

static void draw_room(
    const Cell_t *room,
    const Cell_t *player,
    int center_x,
    int center_y)
{
    
    int screen_x = map_to_screen_x(
        room->position.x,
        player->position.x,
        center_x
    );
    int screen_y = map_to_screen_y(
        room->position.y,
        player->position.y,
        center_y
    );

    int height, width;
    getmaxyx(stdscr, height, width);

    if(
        screen_x < 0 || screen_y >= height ||
        screen_y < 0 || screen_x >= width
    ) return;

    cchar_t symbol = get_symbol(player, room);
    mvadd_wch(screen_y, screen_x, &symbol);
    refresh();
}

void draw_map(const Cell_t *player){
    erase();

    int height, width;
    getmaxyx(stdscr, height, width);

    int center_x = width >> 1;
    int center_y = height >> 1;
    char symbol = ' ';

    Cell_t *room, *tmp;
    HASH_ITER(hh, map->cells, room, tmp){
        draw_room(room, player, center_x, center_y);
    }

    uint16_t size = component_find(player)->component_size;

    mvprintw(0,0,"Position: (%d,%d) Rooms: %u", player->position.x, player->position.y, map->cellc);
    mvprintw(1,0,"Room size: %d", size);
    mvprintw(2,0,"Current room:%u", player->id);
    mvprintw(3,0,"Symbol:%c", symbol);

    mvaddch(18,33, (player->neighbor[DIR_WEST]) ? '<' : ' ');
    mvaddch(18,107, (player->neighbor[DIR_EAST]) ? '>' : ' ');
    mvaddch(8, 67, (player->neighbor[DIR_NORTH]) ? '^' : ' ');
    mvaddch(28,67, (player->neighbor[DIR_SOUTH]) ? 'v' : ' ');

}

void visuals_init(void){
    setlocale(LC_ALL, "");  // Enable the system's locale
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    // Init colors
    start_color();
    use_default_colors();

    init_pair(1, COLOR_RED, -1);

    draw_map(map->cells);
}

// Let's add background and foreground

void draw_player(Cell_t *player, char glyph){

    int x = player->position.x;
    int y = player->position.y;

    mvaddch(y,x,glyph);
}
