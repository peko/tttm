/////////////////////
// TETRA TREE MESH //
/////////////////////

#ifndef TTM_INCLUDE
#define TTM_INCLUDE


typedef struct ttm {
    kvec_t(pnt)  points;     // points
    kvec_t(ttt)  triangles;  // triangles
    ttt         *root;       // root triangle
    mesh        *shape_mesh; // associated regular mesh
} ttm;

ttm* ttm_init(mesh *msh);

void ttm_subdivide (ttm *tm);
void ttm_save_ply  (ttm *tm, char *name);
void ttm_free      (ttm *tm);

#endif