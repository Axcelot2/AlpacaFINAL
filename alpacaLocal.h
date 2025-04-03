// This header file contains the declarations of all functions used by the main function that interact with local directories and files.
#include "dirStruct.h"

#define MAX_CONTENT_CHAR 500
// Declarations of all functions. See alpacaLocal.c for definitions.
int indexDir(struct dirStruct dirArr[MAX_DIR]);
int indexFile(struct dirStruct dirArr[MAX_DIR]);
int listDir(struct dirStruct dirArr[MAX_DIR]);
int listFile(struct dirStruct dirArr[MAX_DIR], int dirIndex);
void createDir(struct dirStruct dirArr[MAX_DIR]);
int createFile(struct dirStruct dirArr[MAX_DIR], char response[MAX_RESPONSE]);
int printFileContent(struct dirStruct dirArr[MAX_DIR]);