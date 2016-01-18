
#include "ttt.h"                                                                                                                                          

#include <stdio.h>
#include <string.h>
#include <malloc.h>


////////////////////////
// TRIANGLE UTILITIES //
////////////////////////

/*

OBJECTS

pnt                         - simple vector
    x, y                    - coordinates

parray                      - dynamic array of void pointers
    v                       - vectors
    used                    - current length
    size                    - allocated size
parray_init   (a, size)     - alocate array
parray_insert (a, pnt )     - insert pnt to array
parray_free   (a      )     - free memory

tri                         - triangle struct
    a, b, c                 - vertex indices

ttt                         - traingle tetra-tree node
    t                       - triangle
    c[4]                    - children
    d                       - linked data

list                        - linked list of void pointers
     е                      - value
     next                   - next element
list_insert      (l, v)     - insert new value
list_free        (l   )     - free list only, keep values 
list_free_values (l   )     - free values and list itself

*/


// convert point to barycentric coordinate system
// static pnt c2b(pnt p, tri t) {
//    pnt pc = (pnt){p.x - t.c.x, p.y - t.c.y};
//    double d = (t.bc.y * t.ac.x - t.bc.x * t.ac.y);
//    double u = (t.bc.y *   pc.x - t.bc.x *   pc.y)/d;
//    double v = (t.ac.x *   pc.y - t.ac.y *   pc.x)/d;
//    return (pnt){u, v};
//    // double w = 1 - u - v; // third coordinate
// }

/////////////////////////
// TRIANGLE TETRA TREE //
/////////////////////////

ttt* ttt_init_cr(pnt center, double radius) {
    ttt* t = (ttt*) malloc(sizeof(ttt));
    t->points = parray_init(256);
    t->root   = tt_init_cr(t->points, center, radius);
    return t;
} 

////////////////////
// TETRA TRIANGLE //
////////////////////

tt* tt_init_p(int a, int b, int c) {
    
    tt* t = malloc(sizeof(tt));

    t->t.a = a;
    t->t.b = b;
    t->t.c = c;

    //children
    for (int i=0; i<4; i++) t->c[i] = NULL;
    t->m = NULL;

    return t;
}

// q = sqrt 3
// a = Rq   = rq/2
// r = R/2  = qa/6 = a/2q = h/3
// R = a/q  = h2/3
// h = aq/2
// p = 3a

// TTT node by center and inner radius
tt* tt_init_cr(parray* points, pnt center, double radius) {

    int a, b, c;

    a = parray_insert(points, (pnt){center.x               , center.y + 2.0*radius});
    b = parray_insert(points, (pnt){center.x - SQRT3*radius, center.y -     radius});
    c = parray_insert(points, (pnt){center.x + SQRT3*radius, center.y -     radius});

    return tt_init_p(a, b, c);
}

// Subdivide
//       a         
//      /1\     0: 00   
//     f - e    1: 01   
//    /2\0/3\   2: 10   
//   b---d---c  3: 11

tt** tt_subdivide(tt* t, parray* p) {
    
    // middle points
    int a, b, c,
        d, e, f;

    a = t->t.a;
    b = t->t.b;
    c = t->t.c;

    d = parray_insert(p, (pnt){
        (p->points[b].x + p->points[c].x)/2.0,
        (p->points[b].y + p->points[c].y)/2.0});
    e = parray_insert(p, (pnt){
        (p->points[c].x + p->points[a].x)/2.0,
        (p->points[c].y + p->points[a].y)/2.0});
    f = parray_insert(p, (pnt){
        (p->points[a].x + p->points[b].x)/2.0,
        (p->points[a].y + p->points[b].y)/2.0});
    
    // children
    t->c[0] = tt_init_p( d, f, e );
    t->c[1] = tt_init_p( a, e, f );
    t->c[2] = tt_init_p( b, f, d );
    t->c[3] = tt_init_p( c, d, e );

    return t->c;
}

// free tree of triangles

void tt_free(tt* t){
    if(t == NULL) return;
    for (int i=0; i<4; i++) if(t->c[i]!=NULL) tt_free(t->c[i]);
    free(t);
}

//////////////////
// POINTS ARRAY //
//////////////////

parray* parray_init(int size) {
    parray* a = malloc(sizeof(parray)); 
    a->points = (pnt*) malloc(size * sizeof(pnt));
    a->used = 0;
    a->size = size;
    return a;
}

int parray_insert(parray* a, pnt p) {
    if (a->used == a->size) {
        a->size *= 2;
        a->points = (pnt *)realloc(a->points, a->size * sizeof(pnt));
    }
    a->points[a->used++] = p;
    return a->used-1;
}

void parray_free(parray* a) {
    free(a->points);
    a->points = NULL;
    a->used = a->size = 0;
    free(a);
}

////////////
// TTLIST //
////////////

tlist* tlist_insert(tlist* l, tri triangle) {
    tlist* head = malloc(sizeof(tlist));
    head->next = l;
    head->triangle = triangle;
    return head;
}

void tlist_free(tlist* l) {
    tlist *next, *cur;
    cur = l;
    while(cur) {
        next = cur->next;
        free(cur);
        cur = next;
    }
}

////////////////////
// TRIANGLE UTILS //
////////////////////

// cross product
static inline double crs(pnt a, pnt b) {
    return a.x*b.y - a.y*b.x;
}

// diff pnttors
static inline pnt dif(pnt a, pnt b) {
    return (pnt){a.x-b.x, a.y-b.y};
}

// triangle intersects
// 1) one of points inside
// 2) one of edges intersects

int tri_x_tri(pnt a, pnt b, pnt c, pnt d, pnt e, pnt f) {
    
    if(tri_x_pnt(a, b, c, d) ||
       tri_x_pnt(a, b, c, e) ||
       tri_x_pnt(a, b, c, f)) return 1;
    
    if(tri_x_pnt(d, e, f, a) ||
       tri_x_pnt(d, e, f, b) ||
       tri_x_pnt(d, e, f, c)) return 1;

    if(line_x_line(a, b, d, e) ||
       line_x_line(a, b, d, f) ||
       line_x_line(a, b, e, f) ||
       line_x_line(a, c, d, e) ||
       line_x_line(a, c, d, f) ||
       line_x_line(a, c, e, f) ||
       line_x_line(b, c, d, e) ||
       line_x_line(b, c, d, f) ||
       line_x_line(b, c, e, f)) return 1;

    return 0;
}

// point intersects triangle
// a,b,c - triangle vertices
// p     - point
int tri_x_pnt(pnt a, pnt b, pnt c, pnt p) {
    
    pnt e, f, g;
    double u, v, d;

    e = dif(c, a);
    f = dif(b, a);
    g = dif(p, a);
    
    u = crs(g, e);
    v = crs(f, g);
    d = crs(f, e);
    
    if(d<0) { u = -u; v = -v; d = -d; }
    return u>=0 && v>=0 && (u+v) <= d;
}

// line intersects line
int line_x_line(pnt e, pnt f, pnt g, pnt h) {

    double d, u, v;
    pnt fe, hg, eg;
    
    fe = dif(f, e);
    hg = dif(h, g);
    eg = dif(e, g);
    
    d = crs(fe, hg);
    u = crs(hg, eg);
    v = crs(fe, eg);

    if(d<0) { u =-u; v =-v; d =-d; }

    return 0<=u && u<=d && 0<=v && v<=d;
}



#ifdef TEST

//////////
// TEST //
//////////

int main(){
    ttt* t = ttt_init_cr((pnt){0.0, 0.0}, 1.0);

}
#endif
