/////////////////////
// TETRA TREE MESH //
/////////////////////

#include "ttm.h"

ttm* ttm_init(mesh* msh) {
    
    double w, h, r;
    w = msh->max.x - msh->min.x;
    h = msh->max.y - msh->min.y;
    r = sqrt(w*w+h*h)/2.0;

    pnt c = (pnt){msh->min.x + w/2.0, msh->min.y +h/2.0};

    ttm* tmsh = malloc(sizeof(ttm));

    return tmsh;
}

void ttm_subdivide(ttm *tm){
}

void ttm_save_ply(ttm *tm, char *name) {
    
    char file_name[256];
    sprintf(file_name, "./mesh/%s-ttm.ply", name);

    // WRITE MESH TO PLY

    FILE* f = fopen(file_name, "w");
    
    if(f == NULL) { printf("Can't open file\n"); return; }

    fprintf(f, "ply\n");
    fprintf(f, "format ascii 1.0\n");
    fprintf(f, "element vertex %d\n", 0);
    fprintf(f, "property float x\n");
    fprintf(f, "property float y\n");
    fprintf(f, "property float z\n");
    fprintf(f, "element face %d\n", 0);
    fprintf(f, "property list uchar int vertex_indices\n");
    fprintf(f, "end_header\n");

    fclose(f);    

}

void ttm_free(ttm *tmsh) {
}

