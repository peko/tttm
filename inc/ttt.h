/////////////////////////
// TETRA TREE TRIANGLE //
/////////////////////////

#ifndef TTT_INCLUDE
#define TTT_INCLUDE

typedef struct ttt ttt;
typedef struct ttt {
    tri         triangle;
    ttt        *children[4];
    int         subdivided;    
} ttt;

ttt* ttt_init     (tri t);
void ttt_subdivide(ttt *t);
void ttt_free     (ttt *t);

#endif