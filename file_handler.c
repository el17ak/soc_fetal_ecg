#include "file_handler.h"

int **read_matrix(size_t *rows, size_t *cols, const char *file_name){
	int offset;
	char *scan;
	size_t j;
	char line[1024];
	int **matrix = NULL, **tmp;
	FILE *fp = fopen(file_name, "r");

    if(rows == NULL || cols == NULL || file_name == NULL) return NULL;

    *rows = 0;
    *cols = 0;

    if(fp == NULL){
        printf("could not open %s\n", file_name);
        return NULL;
    }


    while(fgets(line, sizeof line, fp)){
        if(*cols == 0){
            // determine the size of the columns based on
            // the first row
            char *scan = line;
            int dummy;
            int offset = 0;
            while(sscanf(scan, "%x%n", &dummy, &offset) == 1){
                scan += offset;
                (*cols)++;
            }
        }

        tmp = realloc(matrix, (*rows + 1) * sizeof *matrix);

        if(tmp == NULL){
            fclose(fp);
            return matrix; // return all you've parsed so far
        }

        matrix = tmp;

        matrix[*rows] = calloc(*cols, sizeof *matrix[*rows]);

        if(matrix[*rows] == NULL){
            fclose(fp);
            if(*rows == 0){ // failed in the first row, free everything
                fclose(fp);
                free(matrix);
                return NULL;
            }

            return matrix; // return all you've parsed so far
        }

        offset = 0;
        scan = line;
        for(j = 0; j < *cols; ++j) {
            if(sscanf(scan, "%x%n", matrix[*rows] + j, &offset) == 1) scan += offset;
            else matrix[*rows][j] = 0; // could not read, set cell to 0
        }

        // incrementing rows
        (*rows)++;
    }

    fclose(fp);

    return matrix;
}
