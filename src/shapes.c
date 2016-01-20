
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <shapefil.h>
#include <proj_api.h>

#include "shapes.h"

void shapes_save_poly(shapes* shp, int shape_id, double zoom) {
    
    char file_name[256];
    sprintf(file_name, "mesh/%s.poly", shp->name_long[shape_id]);

    FILE* f = fopen(file_name, "w");
    if(f == NULL) { printf("Can't write file\n%s\n",file_name); return; }
    
    int l = shp->length[shape_id];
    // First line: <# of vertices> <dimension (must be 2)> <# of attributes> <# of boundary markers (0 or 1)>
    fprintf(f, "%d 2 0 0\n", l);
    for(int i=0; i<l; i++) {
        // Following lines: <vertex #> <x> <y> [attributes] [boundary marker]
        fprintf(f, "%d %f %f\n", i, shp->prX[shape_id][i]/zoom, shp->prY[shape_id][i]/zoom);
    }
    
    // One line: <# of segments> <# of boundary markers (0 or 1)>
    fprintf(f, "%d 0\n", shp->length[shape_id]);
    int sp = 1;
    int close_id = 0;
    for(int i=0; i<shp->length[shape_id]; i++) {
        // Following lines: <segment #> <endpoint> <endpoint> [boundary marker]
        if(i == l-1 || shp->parts[shape_id][sp] == i+1) {
            fprintf(f, "%d %d %d\n", i, i, close_id); // close part
            close_id = i+1;                           // new start point id
            sp++;
        } else {
            fprintf(f, "%d %d %d\n", i, i, i+1);
        }
    }
    // One line: <# of holes>
    fprintf(f, "0\n");
    // Following lines: <hole #> <x> <y>
    
    // Optional line: <# of regional attributes and/or area constraints>
    // Optional following lines: <region #> <x> <y> <attribute> <maximum area>
    fclose(f);
}

// LOAD SHP / DBF

shapes* shapes_load(char* filename){
    
    shapes* shp = (shapes*) malloc(sizeof(shapes));

    char buf[256];
    
    strcpy(buf,filename);
    strcat(buf,".shp");
    shapes_load_shp(shp, buf);
    
    strcpy(buf,filename);
    strcat(buf,".dbf");
    shapes_load_dbf(shp, buf);

    return shp;
}

// LOAD DBF

void shapes_load_dbf(shapes* shp, char* filename){

    DBFHandle   hDBF;
    int     *panWidth, i, iRecord;
    char    szFormat[32], szField[1024];
    char    ftype[15], cTitle[32], nTitle[32];
    int     nWidth, nDecimals;
    int     cnWidth, cnDecimals;
    DBFHandle   cDBF;
    DBFFieldType    hType,cType;
    int     ci, ciRecord;

    hDBF = DBFOpen( filename, "rb" );
    if( hDBF == NULL ) {
        printf( "DBFOpen(%s,\"r\") failed.\n", filename );
        return;
    }

    // printf ("Info for %s\n", filename);

    // i = DBFGetFieldCount(hDBF);
    // printf ("%d Columns,  %d Records in file\n",i,DBFGetRecordCount(hDBF));
    
    // panWidth = (int *) malloc( DBFGetFieldCount( hDBF ) * sizeof(int) );
    // for( int i = 0; i < DBFGetFieldCount(hDBF); i++ ) {
    //     DBFFieldType    eType;
    //     char szTitle[256];
    //     eType = DBFGetFieldInfo( hDBF, i, szTitle, &nWidth, &nDecimals );
    //     printf( "%4d: %10s %c", i, szTitle, i%4 ? '|':'\n');
    // }
    // printf("\n");
    
    shp->name_long = malloc(DBFGetRecordCount(hDBF) * sizeof(char*));
    int nl_i = DBFGetFieldIndex( hDBF, "name_long");
    for(int rec = 0; rec < DBFGetRecordCount(hDBF); rec++ ) {
        char* name_long = (char *) DBFReadStringAttribute(hDBF, rec, nl_i);
        shp->name_long[rec] = malloc(strlen(name_long)+1);
        strcpy(shp->name_long[rec], name_long);
        printf("%s\n", shp->name_long[rec]);
    }
    DBFClose( hDBF );

}

// LOAD SHP

void shapes_load_shp(shapes* shp, char* filename){

    SHPHandle hSHP;
    double  adfMinBound[4], adfMaxBound[4];
    
    // Read file 
    hSHP = SHPOpen( filename, "rb" );

    if( hSHP == NULL ) {
        printf( "Unable to open:%s\n", filename );
        return;
    }
            
    // Print shape bounds
    SHPGetInfo( hSHP, &shp->count, &shp->type, adfMinBound, adfMaxBound );

    printf( "Shapefile Type: %s   # of Shapes: %d\n\n",
            SHPTypeName( shp->type ), shp->count );
    
    printf( "File Bounds: (%.15g,%.15g,%.15g,%.15g)\n"
            "         to  (%.15g,%.15g,%.15g,%.15g)\n",
            adfMinBound[0], 
            adfMinBound[1], 
            adfMinBound[2], 
            adfMinBound[3], 
            adfMaxBound[0], 
            adfMaxBound[1], 
            adfMaxBound[2], 
            adfMaxBound[3] );

    // Iterate through entries
    const char  *pszPlus;
    

    shp->X           = (double **) calloc(shp->count, sizeof(double*));
    shp->Y           = (double **) calloc(shp->count, sizeof(double*));
    shp->Z           = (double **) calloc(shp->count, sizeof(double*));
    shp->prX         = (double **) calloc(shp->count, sizeof(double*));
    shp->prY         = (double **) calloc(shp->count, sizeof(double*));
    shp->prZ         = (double **) calloc(shp->count, sizeof(double*));
    
    shp->cX          = (double*) calloc(shp->count, sizeof(double));
    shp->cY          = (double*) calloc(shp->count, sizeof(double));

    shp->length      = (int* ) calloc(shp->count, sizeof(int ));
    shp->parts_count = (int* ) calloc(shp->count, sizeof(int ));
    shp->parts       = (int**) calloc(shp->count, sizeof(int*));
    
    for( int i = 0; i < shp->count; i++ ) {

        int j, iPart;
        SHPObject   *psShape;

        psShape = SHPReadObject( hSHP, i );
        // SHPProject( psShape, old_prj, new_prj );

        if( psShape == NULL ) {
            fprintf( stderr,
                     "Unable to read shape %d, terminating object reading.\n",
                    i );
            break;
        }

        if( psShape->nParts > 0 && psShape->panPartStart[0] != 0 ) {
            fprintf( stderr, "panPartStart[0] = %d, not zero as expected.\n",
                     psShape->panPartStart[0] );
        }
        
        shp->length[i]      = psShape->nVertices;
        shp->parts_count[i] = psShape->nParts;


        shp->X[i]    = (double *)calloc(psShape->nVertices, sizeof(double));
        shp->Y[i]    = (double *)calloc(psShape->nVertices, sizeof(double));
        shp->Z[i]    = (double *)calloc(psShape->nVertices, sizeof(double));

        shp->prX[i] = (double *)calloc(psShape->nVertices, sizeof(double));
        shp->prY[i] = (double *)calloc(psShape->nVertices, sizeof(double));
        shp->prZ[i] = (double *)calloc(psShape->nVertices, sizeof(double));

        shp->parts[i] = (int *)calloc(psShape->nParts, sizeof(int));
                
        for (j =0; j< psShape->nParts; j++) shp->parts[i][j] = psShape->panPartStart[j];
        for( j = 0, iPart = 1; j < psShape->nVertices; j++ ) {
            shp->X[i][j] = psShape->padfX[j];
            shp->Y[i][j] = psShape->padfY[j];
            shp->Z[i][j] = psShape->padfZ[j];
        }

        shp->cX[i] = (psShape->dfXMax + psShape->dfXMin)/2.0;
        shp->cY[i] = (psShape->dfYMax + psShape->dfYMin)/2.0;

        memcpy(shp->prX[i], shp->X[i], shp->length[i]*sizeof(double));
        memcpy(shp->prY[i], shp->Y[i], shp->length[i]*sizeof(double));
        memcpy(shp->prZ[i], shp->Z[i], shp->length[i]*sizeof(double));

        SHPDestroyObject( psShape );
    }
    SHPClose( hSHP );
}

// SHAPES PROJECT

void shapes_project(shapes* shp, double lng, double lat) {


    char* pj_old_str = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
    char pj_new_str[256];
    sprintf(pj_new_str, "+proj=laea +lat_0=%f +lon_0=%f +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs",lat,lng);
    
    projPJ old_prj = pj_init_plus(pj_old_str);
    projPJ new_prj = pj_init_plus(pj_new_str);

    for(int i=0; i<shp->count; i++) {
        
        memcpy(shp->prX[i], shp->X[i], shp->length[i]*sizeof(double));
        memcpy(shp->prY[i], shp->Y[i], shp->length[i]*sizeof(double));
        memcpy(shp->prZ[i], shp->Z[i], shp->length[i]*sizeof(double));
        
        for (int j=0; j<shp->length[i]; j++) {
            shp->prX[i][j] *= DEG_TO_RAD;
            shp->prY[i][j] *= DEG_TO_RAD;
            shp->prZ[i][j]  = 0; 
        }
        pj_transform(old_prj, new_prj, shp->length[i], 0, 
                 shp->prX[i],
                 shp->prY[i], NULL);
    }
    // printf("%f, %f\n", shp->prX[0][0]/4e6, shp->prY[0][0]/4e6);
    pj_free(new_prj);
    pj_free(old_prj);
}

// 

void shapes_project_shape(shapes* shp, int shape_id, double lng, double lat) {


    char* pj_old_str = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
    char pj_new_str[256];
    sprintf(pj_new_str, "+proj=laea +lat_0=%f +lon_0=%f +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs",lat,lng);
    
    projPJ old_prj = pj_init_plus(pj_old_str);
    projPJ new_prj = pj_init_plus(pj_new_str);

    memcpy(shp->prX[shape_id], shp->X[shape_id], shp->length[shape_id] * sizeof(double));
    memcpy(shp->prY[shape_id], shp->Y[shape_id], shp->length[shape_id] * sizeof(double));
    memcpy(shp->prZ[shape_id], shp->Z[shape_id], shp->length[shape_id] * sizeof(double));
    
    for (int j=0; j<shp->length[shape_id]; j++) {
        shp->prX[shape_id][j] *= DEG_TO_RAD;
        shp->prY[shape_id][j] *= DEG_TO_RAD;
        shp->prZ[shape_id][j]  = 0; 
    }

    pj_transform(old_prj, new_prj, shp->length[shape_id], 0, 
             shp->prX[shape_id],
             shp->prY[shape_id], NULL);

    // printf("%f, %f\n", shp->prX[0][0]/4e6, shp->prY[0][0]/4e6);
    pj_free(new_prj);
    pj_free(old_prj);
}

// CLEANUP

void shapes_free(shapes* shp) {

    for(int i=0; i<shp->count; i++) {

        free(shp->X[i]);
        free(shp->Y[i]);
        free(shp->Z[i]);
        
        free(shp->prX[i]);
        free(shp->prY[i]);
        free(shp->prZ[i]);

        free(shp->parts[i]);

        // DBF
        free(shp->name_long[i]);
        // free(shape_continent[i]);
        // free(shape_subregion[i]);
    }
    free(shp->X);
    free(shp->Y);
    free(shp->Z);
    
    free(shp->prX);
    free(shp->prY);
    free(shp->prZ);

    free(shp->length);
    free(shp->parts_count);
    free(shp->parts);

    free(shp->cX);
    free(shp->cY);

    //DBF
    free(shp->name_long);
    // free(shape_continent);
    // free(shape_subregion);
    // free(shape_pop_est);

    free(shp);
}   
