/*

    1) Load shape
    2) Reproject
    3) Save poly
    4) Mesh with triangle
    5) Load shape mesh
    6) Remesh with tetra triangles
    7) Save ply mesh

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "shapes.h"
#include "mesh.h"

static void parse_params(int argc, char** argv);

char* shape_name;
double zoom = 1.0e7;

int main(int argc, char** argv) {

    parse_params(argc, argv);

    // LOAD_SHAPES
    shapes* shp = shapes_load(shape_name);

    // FOR EACH COUNTRY 
    for(int s=0; s<shp->count; s++) {
        // REPROJECT
        shapes_project_shape(shp, s, shp->cX[s], shp->cY[s]);
        // SAVE POLY
        shapes_save_poly(shp, s, zoom);
        // MESH WITH TRIANGLE
        mesh_poly_file(shp->name_long[s]);
        // LOAD MESH
        mesh  *msh  = mesh_load(shp->name_long[s]);
        tmesh *tmsh = tmesh_init(msh);
        tmesh_subdivide(tmsh);
        tmesh_subdivide(tmsh);
        tmesh_subdivide(tmsh);
        tmesh_save_ply(tmsh, shp->name_long[s]);
        // mesh_save_sphere_ply(shape_name_long[s], shape_cxX[s], shape_cxY[s], zoom);
        // mesh_regular_triangle_fill(shape_name_long[s]);
        mesh_free(msh);
    }

    shapes_free(shp);

    exit(EXIT_SUCCESS);
}

static void parse_params(int argc, char** argv) {
    
    if(argc != 2) goto usage;
    
    shape_name = argv[1];
    return;

usage:
    printf("Usage:\n");
    printf("shape_mesher meshfile\n");
    exit(EXIT_FAILURE);
 }