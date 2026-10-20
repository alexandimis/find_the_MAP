
#include<stdio.h>
#include <ncurses.h>

#include "map.h"
#include "visuals.h"


extern Map_t *map;

void game_run(){
    Cell_t *player = map->cells;

    visuals_init();

    int input;
    do{
        input = getch();
        Drctn dir;

        switch (input)
        {
        case KEY_UP:
            dir = DIR_NORTH;
            break;
        case KEY_DOWN:
            dir = DIR_SOUTH;
            break;
        case KEY_LEFT:
            dir = DIR_WEST;
            break;
        case KEY_RIGHT:
            dir = DIR_EAST;
            break;
        default:
            break;
        }

        if(player->neighbor[dir]){
            player = cell_find(get_key(adjecent_pos(player, dir)));
        }

        draw_map(player);
    }while(input != 'q');

}

int main(int argc, char *argv[]){

    if(argc < 2){
        printf("Include map seed as argument.\n");
        return 0;
    }
    uint8_t seed = atoi(argv[1]);
    map_generate(seed);

    game_run();

    map_destroy();
    endwin();

    return 0;
}