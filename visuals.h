#ifndef VISUALS_H
#define VISUALS_H

#include "map.h"


// Beautifull UniCode LUT for the hallways
// The 'L' prefix is for Wide chars/Unicodes
static const wchar_t *const hallway_chars[16] = {
    L" ", // 0000 - None
    L"╶", // 0001 - Right
    L"╴", // 0010 - Left
    L"─", // 0011 - Left + Right
    L"╵", // 0100 - Up
    L"└", // 0101 - Up + Right
    L"┘", // 0110 - Up + Left
    L"┴", // 0111 - Up + Left + Right
    L"╷", // 1000 - Down
    L"┌", // 1001 - Down + Right 
    L"┐", // 1010 - Down + Left
    L"┬", // 1011 - Down + Left + Right
    L"│", // 1100 - Down + Up
    L"├", // 1101 - Down + Up + Right
    L"┤", // 1110 - Down + Up + Left
    L"┼"  // 1111 - All directions
};

void draw_map(const Cell_t *player);
void visuals_init(void);
void draw_player(Cell_t *player, char glyph);


#endif