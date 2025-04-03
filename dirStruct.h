// This header file defines the struct used to represent directories.
#define MAX_TITLE_CHAR 20
#define MAX_FILES 50
#define MAX_DIR 10
#define MAX_RESPONSE 4096

struct dirStruct {
    char dirTitle[MAX_TITLE_CHAR + 1]; // + 1 to include null terminator.
    char fileTitle[MAX_FILES][MAX_TITLE_CHAR + 1]; // + 1 to include null terminator.
};