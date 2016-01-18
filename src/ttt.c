////////////////////////
// TETRA TREE TRIAGLE //
////////////////////////

#include "ttt.h"

// a, b, c - indices
ttt* ttt_init(tri t) {
    ttt *tt = malloc(sizeof(ttt));
    tt->triangle = t;
    for(char i=0; i<4; i++) tt->children[i] = NULL;
    tt->subdivided = 0;
    return tt;
}

// p - points array
// c - center
// r - circle radius 

// ttt* ttt_init_by_circle(parray *p, pnt c, double r) {
//     return ttt_init(
//         parray_insert(p, (pnt){c.x        , c.y+2.0*r}),
//         parray_insert(p, (pnt){c.x-SQRT3*r, c.y-    r}),
//         parray_insert(p, (pnt){c.x+SQRT3*r, c.y-    r}));
// }

void ttt_subdivide(ttt *t) {

}

void ttt_free(ttt* t) {
    for(char i=0; i<4; i++) if(t->children[i]) ttt_free(t->children[i]);
    free(t);
}
