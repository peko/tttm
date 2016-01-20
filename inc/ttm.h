/////////////////////
// TETRA TREE MESH //
/////////////////////

#ifndef TTM_INCLUDE
#define TTM_INCLUDE

#include "types.h"
#include "kvec.h"

typedef struct mesh mesh;
typedef struct pnt pnt;
typedef struct ttt ttt;

typedef struct ttm {
    kvec_t(pnt)  points;     // points
    kvec_t(ttt)  triangles;  // triangles
    ttt         *root;       // root triangle
} ttm;

ttm* ttm_init(pnt c, float r);

void ttm_subdivide (ttm *tm);
void ttm_subdivide_by_mesh(ttm *tm, mesh *msh);
void ttm_free      (ttm *tm);

void ttm_save_ply  (ttm *tm, char *name);

#endif