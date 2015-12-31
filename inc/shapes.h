#ifndef SHAPES_H_INCLUDED
#define SHAPES_H_INCLUDED

typedef struct {
        
    int count;          // shape count
    int type;           // type of shapes

    double ** X;        // x coordinates
    double ** Y;        // y coordinates
    double ** Z;        // z coordinates

    double ** prX;      // projected x coodinates 
    double ** prY;      // projected y coodinates
    double ** prZ;      // projected z coodinates

    double* cX;         // center x coordinates
    double* cY;         // center y coordinates

    int *  length;      // length of shape
    int *  parts_count; // parts count 
    int ** parts;       // parts stop-count


    // DBF data

    char** name_long;   // name long
    // char** shape_continent;
    // char** shape_subregion;
    // int*   shape_pop_est;
} shapes;


shapes* shapes_load(char* filename);

void shapes_load_shp      (shapes* shp, char* filename);
void shapes_load_dbf      (shapes* shp, char* filename);
void shapes_project       (shapes* shp, double lng, double lat);
void shapes_project_shape (shapes* shp, int shape_id, double lng, double lat);
void shapes_save_poly     (shapes* shp, int shape_id, double zoom);
void shapes_free          (shapes* shp);

#endif
