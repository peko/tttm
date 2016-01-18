
#ifndef TTT_INCLUDE
#define TTT_INCLUDE

#include "types.h"

#define SQRT3 1.73205080757

// DYNAMIC POINT ARRAY
typedef struct parray {
    pnt*         points;
    unsigned int used;
    unsigned int size;
} parray;

parray* parray_init   (int size);
int     parray_insert (parray* a, pnt p);
void    parray_free   (parray* a);

// TRIANGLE LIST
typedef struct tlist  tlist;
typedef struct tlist {
    tri    triangle;
    tlist *next;
} tlist;

tlist* tlist_insert(tlist* l, tri triangle);
void   tlist_free  (tlist* l);

// TETRA TRIANGLE
typedef struct tt tt;
typedef struct tt {
    tri    triangle;    // triangle
    tt*    children[4]; // children
    tlist* mesh;        // associated mesh triangles intersected with it
} tt;

tt*  tt_init_cr   (parray* points, pnt center, double radius);
tt*  tt_init_p    (int a, int b, int c);
tt** tt_subdivide (tt* t, parray* p);
void tt_free      (tt* t);

int tri_x_tri  (pnt a, pnt b, pnt c, pnt d, pnt e, pnt f);
int tri_x_pnt  (pnt a, pnt b, pnt c, pnt p);
int line_x_line(pnt e, pnt f, pnt g, pnt h);

#endif
