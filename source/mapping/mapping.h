/*
 *  \file  mapping.h
 *  \brief  define functions for mapping from scanning
 *  \author  Michele Simili
 */

#ifndef MAPPING_H
#define MAPPING_H

typedef struct Matrix {
    int     rowSize;
    int     columnSize;
    long int**    matrix;
} Matrix;

typedef struct Matrix_double {
    int     rowSize;
    int     columnSize;
    double**    matrix;
} Matrix_double;


//define functions
//

Matrix createMatrix(int rowSize, int columnSize);
Matrix_double createMatrix_double(int rowSize, int columnSize);


void scann_mapping(int position[2], Matrix map, Matrix pol_coord);


//transform an array of pol_coord in an array of cart_coord
void pol_to_cart(Matrix_double pol_coord, Matrix_double cart_coord);



//insert the cartesian coordinates into a tmp matrix (local)
void insert_points(Matrix map, Matrix_double points);




//insert the tmp matrix into the general mapping matrix
void adjust_position(double position[2], Matrix map, Matrix map_local );
#endif /* ifndef MAPPING_H */
