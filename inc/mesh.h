#ifndef MESH_INCLUDE
#define MESH_INCLUDE

#include "types.h"
#include "kvec.h"

//////////////////
// REGULAR MESH //
//////////////////

typedef struct mesh {

    int  points_count;
    int  triangles_count;
    
    pnt max, min;

    pnt *points;
    tri *triangles;

} mesh;


mesh* mesh_load(char* name);

void mesh_poly_file(char* name);
void mesh_save_ply (mesh* msh, double lng, double lat, double zoom);
void mesh_free     (mesh* msh);

// void mesh_save_sphere_ply(char* name, double lng, double lat, double zoom);
// void mesh_regular_triangle_fill(char* name);
// void mesh_project(double lng, double lat);

#endif