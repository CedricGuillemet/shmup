
// Ship

char shipPositions[13 * 3] = {
-127, 120,   0,
 127,  90,   0,
 -55,  50, -32,
 -55,  50,  32,

-127, -120,   0,
 127,  -90,   0,
 -55,  -50, -32,
 -55,  -50,  32,

 -100, 0, -32,
 -100, 0,  32,

 -90,  40, 0,
 -10,   0, 0,
 -90, -40, 0
};

unsigned char shipTris[14 * 3] = {
    3,2,0, // left wing reactor
    4,6,7, // right wing reactor

    8,10,9, // center reactor 1 
    8,9,12, // center reactor 2

    2,3,1, // left wing inside
    5,7,6, // riht wing inside

   0,1,3,      // top left wing
   7,5,4, // top right wing
   9,10,11, // center top
   9,11,12, // center top 2

   11,10,8, // center bottom
   12,11,8, // center bottom 2
   2,1,0, // bottom left wing
   4,5,6, // bottom right wing
};

unsigned char shipTrisColor[14] = {
19,19,19,19, 8,8,

3,4,3,4, 14,12,14,12
};