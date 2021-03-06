#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#include "mesh.h"
#include "shapes.h"
#include "proj_api.h"

#define SQRT3 1.73205080757

mesh* mesh_init() {
    mesh* msh = malloc(sizeof(mesh));
    kv_init(msh->points);
    kv_init(msh->triangles);
    msh->min = (pnt){DBL_MAX, DBL_MAX};
    msh->max = (pnt){DBL_MIN, DBL_MIN};
    return msh;
}

void mesh_free(mesh *msh) {
    kv_destroy(msh->triangles);
    kv_destroy(msh->points);
    free(msh);
}

///////////////
// MESH POLY //
///////////////

// launch triangle util over poly file

void mesh_poly_file(char* name) { 
    char cmd[256];
    // sprintf(cmd, "./triangle -pzq32.5a.001 \"../mesh/%s.poly\"", name);
    sprintf(cmd, "./triangle -pz \"mesh/%s.poly\"", name);
    printf("%s\n", cmd);
    system(cmd);
}

///////////////
// MESH LOAD //
///////////////

// loader for poly format generated by triangle util

mesh* mesh_load(char* name) {

    mesh* msh = mesh_init();

    char file_name[256];

    printf("LOADING MESH:\n");
    FILE* f;

    // READ RESULTS BACK //

    char*   line = NULL;
    size_t  len  = 0;
    ssize_t read = 0;

    // POINTS //

    sprintf(file_name, "mesh/%s.1.node", name);
    printf("\tLoad points %s", file_name);

    f = fopen(file_name, "r");
    if(f == NULL) { printf("Can't open elements file\n"); goto error;}
    
    // get header    
    read = getline(&line, &len, f);
    int points_count = atoi(line);

    printf("count: %d\n", points_count);
    // msh->points = malloc(msh->points_count * sizeof(pnt));

    int index = -1;
    int i = 0;
    
    while(i++ < points_count && (read = getline(&line, &len, f)) != -1) {
        char* t;
        
        // get index
        t = strtok(line, " ");
        index = atoi(t);
        pnt p = {0.0, 0.0};
        // get x
        t = strtok(NULL, " ");
        p.x = atof(t);
        // get y
        t = strtok(NULL, " ");
        p.y = atof(t);
        kv_push(pnt, msh->points, p);

        if(msh->min.x > p.x) msh->min.x = p.x;
        if(msh->min.y > p.y) msh->min.y = p.y;
        if(msh->max.x < p.x) msh->max.x = p.x;
        if(msh->max.y < p.y) msh->max.y = p.y;

    }
    fclose(f);
    free(line);
    line = NULL;

    // TRIANGLES //
    
    sprintf(file_name, "mesh/%s.1.ele", name);
    printf("\tLoad triangles %s",file_name);
    
    f = fopen(file_name,"r");
    if(f == NULL) { printf("Can't open elements file\n"); goto error; }
    
    // get header    
    read = getline(&line, &len, f);
    int triangles_count = atoi(line);
    printf("count: %d\n", triangles_count);

    index = -1;
    i = 0;
    while(i++ < triangles_count && (read = getline(&line, &len, f)) != -1) {
        char* t;
        // get index
        t = strtok(line, " ");
        index = atoi(t);
        tri tr = {0, 0, 0};
        t = strtok(NULL, " "); tr.a = atoi(t);
        t = strtok(NULL, " "); tr.b = atoi(t);
        t = strtok(NULL, " "); tr.c = atoi(t);
        kv_push(tri, msh->triangles, tr);
    }
    fclose(f);
    
    // cleanup    
    free(line);

    return msh;

error:

    mesh_free(msh);
    return NULL;

}

//////////////
// SAVE PLY //
//////////////

// save mesh data in a standford ply format

void mesh_save_ply(mesh* msh, double lng, double lat, double zoom) {
        
    FILE* f = fopen("mesh.ply", "w");
    
    if(f == NULL) { printf("Can't open file\n"); return; }
    
    fprintf(f, "ply\n");
    fprintf(f, "format ascii 1.0\n");
    fprintf(f, "comment PROJ: +proj=laea +lat_0=%f +lon_0=%f +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs\n",lat,lng);
    fprintf(f, "comment ZOOM: %f\n", zoom);
    fprintf(f, "element vertex %zu\n", msh->points.n);
    fprintf(f, "property float x\n");
    fprintf(f, "property float y\n");
    fprintf(f, "property float z\n");
    fprintf(f, "element face %zu\n", msh->triangles.n);
    fprintf(f, "property list uchar int vertex_indices\n");
    fprintf(f, "end_header\n");

    for(int i=0; i<msh->points.n   ; i++) 
        fprintf(f, 
            "%f %f %f\n", 
            kv_A(msh->points,i).x, 
            kv_A(msh->points,i).y, 
            0.0);

    for(int i=0; i<msh->triangles.n; i++) 
        fprintf(f, "3 %d %d %d\n", 
            kv_A(msh->triangles,i).a, 
            kv_A(msh->triangles,i).b,
            kv_A(msh->triangles,i).c);

    fclose(f);    
}

// static int check_intersection(tri t) {
//     int intersects = 0;
//     for(int i=0; i<msh->triangles_count; i++){
//         int a, b, c;
//         a = msh->triangles[i*3  ];
//         b = msh->triangles[i*3+1];
//         c = msh->triangles[i*3+2];
//         tri st = {
//             {mesh_X[a],mesh_Y[a]},
//             {mesh_X[b],mesh_Y[b]},e
//             {mesh_X[c],mesh_Y[c]}};e
//         if(tri_x_tri(t, st)) return 1;e
//     }
//     return 0;
// } 

// void mesh_regular_triangle_fill(char* name) {

//     char file_name[256];
//     sprintf(file_name,"../mesh/%s-regular.ply", name);

//     // WRITE MESH TO PLY

//     FILE* f = fopen(file_name, "w");
    
//     if(f == NULL) { printf("Can't open file\n"); return; }

//     double n = 100.0;
//     double w = mesh_max_X - mesh_min_X;
//     double h = mesh_max_Y - mesh_min_Y;
//     double m = w > h ? w : h;
//     double s = m / n;
    
//     int c = ceil(w/s)*ceil(h/s);
//     int j = 0;
//     double p[c*2];
//     for(double x=mesh_min_X; x<=mesh_max_X; x+=s) {
//         for(double y=mesh_min_Y; y<=mesh_max_Y; y+=s) {
//             tri t = {{x-s,y-s/2},{x+s, y-s/2},{x,y+s/2}};
//             if(check_intersection(t)) {
//                 // printf("%f %f\n", x, y);  
//                 p[j*2  ] = x;
//                 p[j*2+1] = y;
//                 j++;
//             }
//         }
//     }

//     fprintf(f, "ply\n");
//     fprintf(f, "format ascii 1.0\n");
//     fprintf(f, "element vertex %d\n", j);
//     fprintf(f, "property float x\n");
//     fprintf(f, "property float y\n");
//     fprintf(f, "property float z\n");
//     fprintf(f, "end_header\n");

//     for(;j>=0; j--){
//         fprintf(f, "%f %f %f\n", p[j*2], p[j*2+1], 0.0);
//     }

//     fclose(f);
// }


// void mesh_save_sphere_ply(char* name, double lng, double lat, double zoom) {
    
//     char file_name[256];
//     sprintf(file_name,"../mesh/%s.ply", name);

//     printf("Save sphere mesh\n");

//     if(msh->triangles == NULL) return;

//     // REPROJECT //

//     char pj_old_str[256];
//     sprintf(pj_old_str, "+proj=laea +lat_0=%f +lon_0=%f +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs",lat,lng);
//     char* pj_new_str = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
    
//     projPJ old_prj = pj_init_plus(pj_old_str);
//     projPJ new_prj = pj_init_plus(pj_new_str);

//     // ALLOCATE  SPACE FOR MESH
//     double* mesh_sphere_X = malloc(msh->points_count * sizeof(double));
//     double* mesh_sphere_Y = malloc(msh->points_count * sizeof(double));
//     double* mesh_sphere_Z = malloc(msh->points_count * sizeof(double));

//     // COPY MESH DATA
//     memcpy(mesh_sphere_X, mesh_X, msh->points_count * sizeof(double));
//     memcpy(mesh_sphere_Y, mesh_Y, msh->points_count * sizeof(double));
    
//     // ZOOM MESH BACK
//     for (int j=0; j<msh->points_count; j++) {
//         mesh_sphere_X[j] *= zoom;
//         mesh_sphere_Y[j] *= zoom;
//     }

//     // PROJECT LAEA to WGS84
//     pj_transform(old_prj, new_prj, msh->points_count, 0, 
//                  mesh_sphere_X,
//                  mesh_sphere_Y, NULL);
//     pj_free(new_prj);
//     pj_free(old_prj);
    
//     // CONVERT lat lng to X Y Z //
//     double R = 1.0;
//     double sin_lat, sin_lng, cos_lat, cos_lng;
//     for (int j=0; j<msh->points_count; j++) {
        
//         sin_lat = sin(mesh_sphere_Y[j]);
//         cos_lat = cos(mesh_sphere_Y[j]);
//         sin_lng = sin(mesh_sphere_X[j]);
//         cos_lng = cos(mesh_sphere_X[j]);

//         mesh_sphere_X[j] = R * cos_lat * cos_lng;
//         mesh_sphere_Y[j] = R * cos_lat * sin_lng;
//         mesh_sphere_Z[j] = R * sin_lat;
//     }

//     // WRITE MESH TO PLY

//     FILE* f = fopen(file_name, "w");
    
//     if(f == NULL) { printf("Can't open file\n"); goto clear; }

//     fprintf(f, "ply\n");
//     fprintf(f, "format ascii 1.0\n");
//     fprintf(f, "element vertex %d\n", msh->points_count);
//     fprintf(f, "property float x\n");
//     fprintf(f, "property float y\n");
//     fprintf(f, "property float z\n");
//     fprintf(f, "element face %d\n", msh->triangles_count);
//     fprintf(f, "property list uchar int vertex_indices\n");
//     fprintf(f, "end_header\n");

//     for(int i=0; i<msh->points_count   ; i++) 
//         fprintf(f, 
//             "%f %f %f\n", 
//             mesh_sphere_X[i], 
//             mesh_sphere_Y[i], 
//             mesh_sphere_Z[i]);

//     for(int i=0; i<msh->triangles_count; i++) 
//         fprintf(f, "3 %d %d %d\n", 
//             msh->triangles[i*3  ], 
//             msh->triangles[i*3+1],
//             msh->triangles[i*3+2]);

//     fclose(f);    

// clear:

//     free(mesh_sphere_X);
//     free(mesh_sphere_Y);
//     free(mesh_sphere_Z);

// }

// void mesh_shape(int shape_id) {
    
//     struct triangulateio in, out;

//     if(shape_id>=shapes_count) return;

//     int l = shapes_length[shape_id];
    
//     //////////////
//     // IN MEHSH //
//     //////////////

//     // POINTS
    
//     in.numberofpoints = l;
//     in.numberofpointattributes = 1;
    
//     in.pointlist = (REAL *) malloc(
//         in.numberofpoints * 2 * 
//         sizeof(REAL));
//     in.pointmarkerlist = (int *) malloc(
//         in.numberofpoints * 
//         sizeof(int));
//     in.pointattributelist = (REAL *) malloc(
//         in.numberofpoints * 
//         in.numberofpointattributes * 
//         sizeof(REAL));

//     // SEGMENTS
    
//     in.numberofsegments = l;
//     in.segmentlist = (int *) malloc(
//         in.numberofsegments * 2 *
//         sizeof(int));
//     in.segmentmarkerlist = (int *) malloc(
//         in.numberofsegments *
//         sizeof(int));

//     // Fill regions with data
//     int close_id = 0;
//     int sp = 1;

//     // REGIONS

//     in.numberofregions = shapes_parts_count[shape_id];
//     in.regionlist = (REAL *) malloc((in.numberofregions+1) * 4 * sizeof(REAL));
    
//     in.regionlist[(sp-1)*4  ] = 0.0;
//     in.regionlist[(sp-1)*4+1] = 0.0;
//     REAL avr_c = 0.0;

//     for(int i=0; i<l; i++) {
        
//         in.pointlist[i*2  ] = shapesX[shape_id][i];
//         in.pointlist[i*2+1] = shapesY[shape_id][i];
        
//         // integrate for average 
//         in.regionlist[(sp-1)*4  ] += shapesX[shape_id][i];
//         in.regionlist[(sp-1)*4+1] += shapesY[shape_id][i];
//         avr_c += 1.0;

//         in.pointmarkerlist[i] = sp;
//         in.pointattributelist[i] = 1.0;
        
//         in.segmentlist[i*2] = i; // first point
//         in.segmentmarkerlist[i] = sp;
//         // if last point of part
//         if(i == l-1 || shapes_parts[shape_id][sp] == i+1) { // if next point is start one
//             // average
//             in.regionlist[(sp-1)*4  ] /= avr_c;
//             in.regionlist[(sp-1)*4+1] /= avr_c;
//             in.regionlist[(sp-1)*4+2]  = 1.0;    // regional attribute
//             in.regionlist[(sp-1)*4+3]  = 1.0;    // area contraint
            
//             in.segmentlist[i*2+1] = close_id;    // close path
//             close_id = i+1;                      // new start point id
//             sp++;
//             in.regionlist[(sp-1)*4  ] = 0.0;
//             in.regionlist[(sp-1)*4+1] = 0.0;
//             avr_c = 0.0;
//         } else {
//           in.segmentlist[i*2+1] = i+1;                      // second - next point
//         }
//     }

//     // HOLES

//     //////////////
//     // OUT MESH //
//     //////////////

//     out.pointlist             = (REAL *) NULL; /* Not needed if -N switch used. */
//     out.pointattributelist    = (REAL *) NULL;
//     out.trianglelist          = (int  *) NULL; /* Not needed if -E switch used. */
//     out.triangleattributelist = (REAL *) NULL;
    
//     out.segmentlist           = (int  *) NULL; 
//     out.segmentmarkerlist     = (int  *) NULL;

//     /* Triangulate the points.  Switches are chosen to read and write a  */
//     /*   PSLG (p), preserve the convex hull (c), number everything from  */
//     /*   zero (z), assign a regional attribute to each element (A), and  */
//     /*   produce an edge list (e), a Voronoi diagram (v), and a triangle */
//     /*   neighbor list (n).                                              */

//     // triangulate("pczAevn", &in, &mid, &vorout);

//     triangulate("pza1ABOV", &in, &out, (struct triangulateio *) NULL);

//     msh->triangles_count = out.numberoftriangles;
//     msh->triangles = malloc(msh->triangles_count * 3 * sizeof(int));
//     memcpy(msh->triangles, out.trianglelist, msh->triangles_count * 3 * sizeof(int));


//     msh->points_count = out.numberofpoints;
//     mesh_X    = (double *)malloc(msh->points_count * sizeof(double));
//     mesh_Y    = (double *)malloc(msh->points_count * sizeof(double));
    
//     mesh_pr_X = (double *)malloc(msh->points_count * sizeof(double));
//     mesh_pr_Y = (double *)malloc(msh->points_count * sizeof(double));

//     for (int i = 0; i < msh->points_count; i++) {
//         mesh_X[i] = out.pointlist[i * 2    ];
//         mesh_Y[i] = out.pointlist[i * 2 + 1];
//     }

//     /////////////
//     // CLEANUP //
//     /////////////
    
//     // in
//     free(in.pointlist);
//     free(in.pointattributelist);
//     free(in.pointmarkerlist);

//     free(in.segmentlist);
//     free(in.segmentmarkerlist);

//     free(in.regionlist);

//     //out
//     free(out.pointlist);
//     free(out.pointattributelist);
//     // free(out.pointmarkerlist);
    
//     free(out.trianglelist);
    
//     free(out.segmentlist);
//     free(out.segmentmarkerlist);
// }

// void mesh_project(double lng, double lat) {

//     if(mesh_pr_X == NULL) return;

//     char* pj_old_str = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
//     char pj_new_str[256];
//     sprintf(pj_new_str, "+proj=laea +lat_0=%f +lon_0=%f +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs",lat,lng);
    
//     projPJ old_prj = pj_init_plus(pj_old_str);
//     projPJ new_prj = pj_init_plus(pj_new_str);

//     memcpy(mesh_pr_X, mesh_X, msh->points_count * sizeof(double));
//     memcpy(mesh_pr_Y, mesh_Y, msh->points_count * sizeof(double));
    
//     for (int j=0; j<msh->points_count; j++) {
//         mesh_pr_X[j] *= DEG_TO_RAD;
//         mesh_pr_Y[j] *= DEG_TO_RAD;
//     }
//     pj_transform(old_prj, new_prj, msh->points_count, 0, 
//                  mesh_pr_X,
//                  mesh_pr_Y, NULL);

//     pj_free(new_prj);
//     pj_free(old_prj);
// }

