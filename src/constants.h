// Ship bullet size when testing with enemies
#define PLAYER_BULLET_COLLISION_HE 16

// Ship size when testing enemy bullets of  *other* color
#define SHIP_COLLISION_HE 4

// Ship size when testing enemy bullets of same color
#define SHIP_COLLISION_ACQUIRE_HE 18

// Enemy size when testing collision with ship
#define ENEMY_TO_SHIP_COLLISION_HE 4

// Enemy sizes when testing with ship bullet
static const int ENEMY_COLLISION_HE[1] = {8};

// Enemy bullet size when testing with ship
static const int ENEMY_BULLET_COLLISION_HE[1] = { 6 };

// Jauge increment when acquired
static const int JAUGE_VALUE[1] = {8};

// Invalid enemy index for trails destination
#define INVALID_ENEMY_INDEX 0xFFFF