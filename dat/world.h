#pragma once

enum TileID {
  AIR = 0,
  GROUND = 2,
  WALL = 3,
  SHIP_STAIRS = 4,
  SHELF = 5,
  CHECKOUT_TERMINAL = 10
};

inline char GetTileCharacter(int tile) {
  switch (tile) {
  case AIR:
    return ' ';

  case GROUND:
    return '.';

  case WALL:
    return '|';

  case SHELF:
    return 'S';

  case SHIP_STAIRS:
    return '[';

  case CHECKOUT_TERMINAL:
    return 'C';

  default:
    return '?';
  }
}
