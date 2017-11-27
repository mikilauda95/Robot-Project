//TODO adjust to have a 360 scanning (sins and cosins could be negative)

/*
 *  \file  mapping.c
 *  \brief  define functions for mapping from scanning
 *  \author  Michele Simili
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
/* #include "ev3.h" */
/* #include "ev3_port.h" */
/* #include "ev3_sensor.h" */
//
//max size of the scanning is 510 counting that measurement can be up to 250. 511 just to be sure to always stay inside the matrix
//
#define MAX_DISTANCE 511

// WIN32 /////////////////////////////////////////
/*#ifdef __WIN32__*/

/*#include <windows.h>*/

/*// UNIX //////////////////////////////////////////*/
/*#else*/

/*#include <unistd.h>*/
/*#define Sleep( msec ) usleep(( msec ) * 1000 )*/

/*//////////////////////////////////////////////////*/
/*#endif*/

#define ROW_SIZE 10
#define COL_SIZE 10

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

Matrix createMatrix(int rowSize, int columnSize){
    Matrix temp = {rowSize, columnSize, malloc(columnSize*sizeof(long int *))};

    if (temp.matrix == NULL) {
        /* panic */
    }

    for (int i = 0; i < rowSize; i++) {
        temp.matrix[i] = malloc(rowSize*sizeof temp.matrix[i][0]);

        if (temp.matrix[i] == NULL) {
            /* panic */
        }
    }

    temp.rowSize=rowSize;
    temp.columnSize=columnSize;

    return temp;
}


Matrix_double createMatrix_double(int rowSize, int columnSize){
    Matrix_double temp = {rowSize, columnSize, malloc(columnSize*sizeof(double *))};

    if (temp.matrix == NULL) {
        /* panic */
    }

    for (int i = 0; i < rowSize; i++) {
        temp.matrix[i] = malloc(rowSize*sizeof temp.matrix[i][0]);

        if (temp.matrix[i] == NULL) {
            /* panic */
        }
    }

    temp.rowSize=rowSize;
    temp.columnSize=columnSize;


    return temp;
}


//transform an array of pol_coord in an array of cart_coord
void pol_to_cart(Matrix_double pol_coord, Matrix_double cart_coord){
    double distance, angle;
    double xc, yc;
    for(int i=0; i<pol_coord.rowSize; i++){

        distance=pol_coord.matrix[0][i];
        angle=pol_coord.matrix[1][i]*(M_PI/180);

        xc=distance*cos(angle);
        yc=distance*sin(angle);


        cart_coord.matrix[0][i]=xc;
        cart_coord.matrix[1][i]=yc;
    }
}

//insert the cartesian coordinates into a tmp matrix (local)
void insert_points(Matrix map, Matrix_double points){
    //working with centimeters
    double res;
    double xc, yc;
    int x_i, y_i;
    /*res=510/map.columnSize; //columnSize=rowSize=number of points*/
    for(int i=0; i<points.rowSize; i++){
        xc=points.matrix[0][i]+255; //in order to be at the center of the matrix
        yc=points.matrix[1][i]+255;
        x_i= (int) ((xc/MAX_DISTANCE)*map.rowSize);
        y_i= (int) ((yc/MAX_DISTANCE)*map.columnSize);
        map.matrix[x_i][y_i]=1;
    }
}

//insert the tmp matrix into the general mapping matrix
void adjust_position(double position[2], Matrix map, Matrix map_local ){
    //working with centimeters
    /*double res=255/map_local.columnSize;*/
    double x_start_cord, y_start_cord;
    int xp_i, yp_i;
    int i,j;
    x_start_cord=position[0]-255;
    y_start_cord=position[1]-255;
    xp_i=(int)((x_start_cord/MAX_DISTANCE)*map_local.rowSize);
    yp_i=(int)((y_start_cord/MAX_DISTANCE)*map_local.columnSize);
    printf("xp_i is %d and yp_i is %d\n", xp_i, yp_i);
    for (i = 0; i < map_local.columnSize; ++i) {
        for (j = 0;  j< map_local.rowSize; j++) {
            if (( i+xp_i )>0&&( j+yp_i )>0 &&((i+xp_i)<map.rowSize) && (j+yp_i)<map.columnSize) {
                map.matrix[i+xp_i][j+yp_i]=map_local.matrix[i][j];
            }
        }
    }
}

//last two functions could be squeezed into one function
