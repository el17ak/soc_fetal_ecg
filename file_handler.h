/*
 * file_handler.h
 *
 *  Created on: 7 May 2022
 *      Author: el17a
 */

#ifndef FILE_HANDLER_H_
#define FILE_HANDLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int **read_matrix(size_t *rows, size_t *cols, const char *file_name);

#endif /* FILE_HANDLER_H_ */
