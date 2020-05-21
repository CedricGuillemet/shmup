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

unsigned char shipTriangles[14 * 3] = {
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

unsigned char shipTriangleColors[14] = {
19,19,19,19, 8,8,
3,4,3,4, 14,12,14,12
};

// Enemy size 0
char enemy0Positions[5 * 3] = {
    -63,0,0,
    0,63,0,
    127,0,0,
    0,-63,0,
    40,0,64,
};

unsigned char enemy0Triangles[4 * 3] = {
    3,1,0,
    3,4,1,
    4,2,1,
    3,2,4,
};

unsigned char enemy0TriangleWhiteColors[4] = {
44,40,13,10
};

unsigned char enemy0TriangleBlackColors[4] = {
24,23,4,6
};

// Enemy size 1
char enemy1Positions[7 * 3] = {
    -63,0,0,
    -32,32,0,
    0,40,0,
    63,0,0,
    0,0,64,
    0,-40,0,
    -32,-32,0,
};

unsigned char enemy1Triangles[6 * 3] = {
    0,6,1,
    6,4,1,
    4,2,1,
    4,3,2,
    4,5,3,
    4,6,5,
};

unsigned char enemy1TriangleWhiteColors[6] = {
44,40,14,12,10,8,
};

unsigned char enemy1TriangleBlackColors[6] = {
24,23,2,4,6,8,
};

// Enemy size 2
char enemy2Positions[15 * 3] = {
   -100,-50,0,
    -90,-25,0,
      0,-50,5,
    100,-25,0,
    -10,-100,0,
    
   -100,50,0,
    -90,25,0,
      0,50,5,
    100,25,0,
    -10,100,0,
    
    -20,0,0,
    0,40,0,
    80,0,0,
    0,-40,0,
    10,0,32,
};

unsigned char enemy2Triangles[10 * 3] = {
    2,1,0,
    4,2,0,
    3,2,4,
    
    5, 6, 7,
    5,7,9,
    9,7,8,

    13,11,10,
    13,14,11,
    14,12,11,
    13,12,14,
};

unsigned char enemy2TriangleWhiteColors[10] = {
    14,14,9,
    13,13,10,
    44,40,13,10
};

unsigned char enemy2TriangleBlackColors[10] = {
    7,7,8,
    6,6,8,
    24,23,4,6
};

// Enemy size 3
char enemy3Positions[16 * 3] = {
    -32,0,0,
    0,32,0,
    110,0,0,
    0,-32,0,
    20,0,32,

    50,-80,-30,
    -80,0,0,
    50,80,-30,

    25,-50,10,
    110,-20,5,
    127,-90,30,
    90,-110,5,

    25,50,10,
    110,20,5,
    127,90,30,
    90,110,5,

};

unsigned char enemy3Triangles[12 * 3] = {
    3,1,0,
    3,4,1,
    4,2,1,
    3,2,4,

    3,0,6,
    5,3,6,
    1,7,6,
    0,1,6,
    /*
    1,2,7,
    3,5,2,
    */
    //wings
    10,9,8,
    10,8,11,

    12,13,14,
    15,12,14,
};

unsigned char enemy3TriangleWhiteColors[12] = {
44,40,13,10,
11,13,
14,12,

//9,10,
10,14,
14,10
};

unsigned char enemy3TriangleBlackColors[12] = {
24,23,4,6,
5,7,
6,4,

//4,5,
5,8,
8,5,
};