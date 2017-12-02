#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include "mapping.h"

#define TMPMAP_MAX 40

int main(){
    //let's debug it




    //initialize the polar coordinates matrix
    Matrix_double pol_coord;
    int rowSize;
    int i,j = 0;

    FILE *f_id;
    f_id = fopen("pol_coordinates.txt", "r");
    if (f_id<0) {
        printf("ERROR\n");
    }
    else {
        fscanf (f_id, "%d\n", &rowSize);
        printf("%d\n", rowSize);
        pol_coord=createMatrix_double(rowSize, 2);
        /*pol_coord.matrix[0][0]=3.7;*/
        /*printf("%lf\n", pol_coord.matrix[0][0]);*/

        for (int i = 0; i < pol_coord.rowSize; ++i) {
            fscanf(f_id, "%lf ", &pol_coord.matrix[0][i]);
            /*printf("%lf ", pol_coord.matrix[0][i]);*/
        }
        printf("\n");
        for (i = 0; i < pol_coord.rowSize; ++i) {
            fscanf(f_id, "%lf ", &pol_coord.matrix[1][i]);
            /*printf("%lf ", pol_coord.matrix[1][i]);*/
        }
        printf("\n");

        fclose(f_id);
    }

        printf("\n");

    //else error in opening the file

    //initialize the cartesian coordinates matrix
    Matrix_double cart_coord;
    cart_coord=createMatrix_double(pol_coord.rowSize, pol_coord.columnSize);
    pol_to_cart(pol_coord, cart_coord);
    for (i = 0; i < cart_coord.columnSize; ++i) {
        for (j = 0; j < cart_coord.rowSize; ++j) {
            printf("%lf ", cart_coord.matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    printf("\n");


    //initialize the temporary matrix
    Matrix tmp_map;
    tmp_map=createMatrix(TMPMAP_MAX, TMPMAP_MAX);
    insert_points(tmp_map, cart_coord);
    for (i = 0; i < tmp_map.columnSize; ++i) {
        for (j = 0; j < tmp_map.rowSize; ++j) {
            printf("%d", tmp_map.matrix[i][j]);
        }
        printf("\n");
    }

    //initialize the map matrix
    Matrix map;
    map=createMatrix(TMPMAP_MAX*2,TMPMAP_MAX*2);
    double position[2];
    position[0]=500;
    position[1]=500;
    adjust_position(position, map, tmp_map);

    for (i=map.columnSize-1; i>0 ; i--) {
        for (j = 0; j < map.rowSize; ++j) {
            printf("%d", map.matrix[i][j]);
        }
        printf("\n");
    }

    return 1;
}
