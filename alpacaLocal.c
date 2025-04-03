// This file defines all the functions used by the main function that involve manipulation local directories/files.
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include "alpacaOllama.h"

// Defines function for directory creation based on user platform.
#ifdef _WIN32
    #define MKDIR(path) mkdir(path)
#else // Assumes UNIX based OS if not Windows.
    #define MKDIR(path) mkdir(path, 0777)
#endif

int indexFile(struct dirStruct dirArr[MAX_DIR]) {

    for (int i = 0; i < MAX_DIR; i++) {
        if (dirArr[i].dirTitle[0] != '\0') {
            int dirIndex = i;
            struct dirent *fileent;
            struct stat filest;
            char dirPath[MAX_TITLE_CHAR + 3];

            snprintf(dirPath, sizeof(dirPath), "./%s", dirArr[dirIndex].dirTitle);

            DIR *fptr = opendir(dirPath);

            if (fptr == NULL) {
                printf("Error indexing files.\n\n");
                return 1;
            }

            int fileIndex = 0;

            while ((fileent = readdir(fptr)) != NULL) {
                char filePath[MAX_TITLE_CHAR*2 + 4];
                snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, fileent->d_name);
                if (stat(filePath, &filest) == 0) {
                    if (!S_ISREG(filest.st_mode)) {
                        continue;
                    }
                    if (strcmp(fileent->d_name, ".") == 0 || strcmp(fileent->d_name, "..") == 0) {
                        continue;
                    }
                    strncpy(dirArr[dirIndex].fileTitle[fileIndex], fileent->d_name, sizeof(dirArr[dirIndex].fileTitle[fileIndex]) - 1);
                    fileIndex++;
                } else {
                    printf("Error indexing pre-existing files.\n\n");
                    return 1;
                }
            }
        }
    }
    
    return 0;
}

// This function indexes all directories that exist in the same directory as the program.
int indexDir(struct dirStruct dirArr[MAX_DIR]) {

    struct dirent *dirent;
    struct stat st;

    DIR *dptr = opendir("./");
    
    // The function prints an error message, returns 1, and terminates if the pointer for the local directory fails to initialize.
    if (dptr == NULL) {
        printf("Error indexing directories.\n\n");
        return 1;
    }

    int dirIndex = 0;
    char prevDir[] = {'.', '.'}; // This would be the directory name of the preceding directory.

    // Reads all files/directories until there are none left.
    while ((dirent = readdir(dptr)) != NULL) {
        if (stat(dirent->d_name, &st) == 0) {
            // We use stat functions to ensure only directories will be indexed.
            if (S_ISDIR(st.st_mode)) {
                // This prevents the current directory and the previous directory from being indexed.
                if (dirent->d_name[0] != '.' && strcmp(dirent->d_name, prevDir) != 0) {
                    // We -1 from the amount of memory allocated or the title to preserve the null terminator.
                    strncpy(dirArr[dirIndex].dirTitle, dirent->d_name, sizeof(dirArr[dirIndex].dirTitle) - 1);
                    // The function increments dirIndex to move to the next directory in the directory struct.
                    dirIndex++;
                }
            }

        // The function prints an error message and return 1 if there was an error with stat() that caused it to return non-0.
        // Otherwise the function returns 0 upon successful execution.
        } else {
            printf("Error verifying pre-existing directories.\n\n");
            return 1;
        }
    }
    return 0;
}

// This function creates a new directory in the same directory as the program, with user input as the name.
void createDir(struct dirStruct dirArr[MAX_DIR]) {
    // Checks if the maximum amount of directories already exists. Prints error message of so.
    if (dirArr[MAX_DIR - 1].dirTitle[0] != '\0') {
        printf("You have reached the maximum amount of directories. (%d)\n\n", MAX_DIR);

    } else {
        // Scans for first available struct.
        for (int i = 0; i < MAX_DIR; i++) {
            if (dirArr[i].dirTitle[0] == '\0') {

                printf("Type in the name of the directory: ");
                fgets(dirArr[i].dirTitle, sizeof(dirArr[i].dirTitle), stdin);

                // fgets() collects the new line character that will be inputted by the user as they hit ENTER.
                // We remove the new line character from the title.
                dirArr[i].dirTitle[strcspn(dirArr[i].dirTitle, "\n")] = '\0';

                // The directory is created in the same directory as the program, therefore, no path is needed beyond the title of the directory.
                if (MKDIR(dirArr[i].dirTitle) == 0) {
                    printf("Directory '%s' created successfully.\n\n", dirArr[i].dirTitle);
                    break;

                // Prints error message of directory creation failed.
                } else {
                    printf("Error creating directory, directory may already exist.\n\n");
                    // indexDir() should not index a failed directory. This prevents that.
                    dirArr[i].dirTitle[0] = 0;
                    break;
                }
            }
        }
    }
}

// This function prints all the indexed directories. Note that it DOES NOT scan and re-indexes directories, but simply prints what is already indexed.
int listDir(struct dirStruct dirArr[MAX_DIR]) {
    int dirExist = 0;
    printf("--- List of Available Categories ---\n\n");
    for (int i = 0; i < MAX_DIR; i++) {
        if (dirArr[i].dirTitle[0] != '\0') {
            printf("%s[%d]\n", dirArr[i].dirTitle, i + 1); // i + 1 because while the array is 0-indexed, the user will start counting from 1, not 0.
            dirExist = 1;
        }
    }

    // The function will return 0 once it finishes printing all indexed directories. It will return 1 instead if no directories are indexed. (dirExist == 0)
    if (dirExist == 1) {
        printf("\n--- End of Category List ---\n\n");
        return 0;
    } else {
        printf("\n--- No Categories Found ---\n\n");
        return 1;
    }
}

int listFile(struct dirStruct dirArr[MAX_DIR], int dirIndex) {
    int fileExist = 0;
    printf("\n--- List of Files in %s ---\n\n", dirArr[dirIndex].dirTitle);
    for (int i = 0; i < MAX_FILES; i++) {
        if (dirArr[dirIndex].fileTitle[i][0] != '\0') {
            printf("%s[%d]\n", dirArr[dirIndex].fileTitle[i], i + 1);
            fileExist = 1;
        }
    }
    if (fileExist == 1) {
        printf("\n--- End of File List ---\n\n");
        return 0;
    } else {
        printf("\n--- No Files Found in %s ---\n\n", dirArr[dirIndex].dirTitle);
        return 1;
    }
}

// This function creates a text file in the user-specified directory with the user-specified title and content.
int createFile(struct dirStruct dirArr[MAX_DIR], char response[MAX_RESPONSE]) {
    
    // listDir() returns 1 if no directories exist, therefore, we can use that as a prompt for an warning message for the user.
    if (listDir(dirArr) != 0) {
        printf("No directories exist. Please create one before creating a file.\n\n");
        return 1;
    }

    // The user decides which directory will hold the new file.
    int selectDir;
    printf("Which of these directories does the file belong to?: ");
    scanf("%d", &selectDir);
    getchar(); // Buffer overflow protection for fgets().
    selectDir--; // selectDir is decremented because the user's specified index is 1-indexed, but the array in the program is 0-indexed.

    // Input validation.
    if (dirArr[selectDir].dirTitle[0] == '\0' || selectDir < 0 || selectDir >= MAX_DIR) {
        printf("Invalid directory index entered. Please only enter a directory listed.\n\n");
        return 1;

    } else {
 // MAX_CONTENT_CHAR + 1 to include the null terminator of the string.
        int fileIndex;

        // Scans for first available file title index.
        for (int i = 0; i < MAX_FILES; i++) {
            if (dirArr[selectDir].fileTitle[i][0] == '\0') {
                fileIndex = i;
                break;
            }
        }

        printf("Type in name of text file: ");
        fgets(dirArr[selectDir].fileTitle[fileIndex], sizeof(dirArr[selectDir].fileTitle[fileIndex]), stdin);

        // Remove new line character from file title.
        dirArr[selectDir].fileTitle[fileIndex][strcspn(dirArr[selectDir].fileTitle[fileIndex], "\n")] = '\0';

        // Both directory and file titles can only have a maximum of MAX_TITLE_CHAR amount of characters.
        // Both the null terminator and the path seperator '/' are 1 character each.
        // Since the file created is within a directory located in the same directory as the program, 2*MAX_TITLE_CHAR + 2 is enough memory for the file path.
        char filePath[2*MAX_TITLE_CHAR + 2];
        snprintf(filePath, sizeof(filePath), "%s/%s", dirArr[selectDir].dirTitle, dirArr[selectDir].fileTitle[fileIndex]);
        
        // File writing.
        FILE *fptr;
        fptr = fopen(filePath, "w");

        // Prints error message if file creation failed.
        if (fptr == NULL) {
            printf("Error creating file.\n\n");
            fclose(fptr);
            return 1;
        } else {
            fprintf(fptr, "%s", response);
        }

        // Closing file and printing success message.
        fclose(fptr);
        printf("File '%s' created.\n\n", dirArr[selectDir].fileTitle[fileIndex]);
        return 0;
    }
}

int printFileContent(struct dirStruct dirArr[MAX_DIR]) {
    int dirIndex, fileIndex;

    if (listDir(dirArr) == 1) {
        return 1;
    }

    printf("Select a directory: ");
    scanf("%d", &dirIndex);
    getchar();
    dirIndex--;

    if (dirIndex < 0 || dirIndex >= MAX_DIR || dirArr[dirIndex].dirTitle[0] == '\0') {
        printf("Invalid directory selected.\n\n");
        return 1;
    }

    if (listFile(dirArr, dirIndex) == 1) {
        return 1;
    }

    printf("Select a file: ");
    scanf("%d", &fileIndex);
    getchar();
    fileIndex--;

    if (fileIndex < 0 || fileIndex >= MAX_FILES || dirArr[dirIndex].fileTitle[fileIndex][0] == '\0') {
        printf("Invalid file selected.\n\n");
        return 1;
    }

    char filePath[2 * MAX_TITLE_CHAR + 2];
    snprintf(filePath, sizeof(filePath), "%s/%s", dirArr[dirIndex].dirTitle, dirArr[dirIndex].fileTitle[fileIndex]);

    FILE *fptr = fopen(filePath, "r");
    if (fptr == NULL) {
        printf("Error opening file.\n\n");
        return 1;
    }

    printf("\n--- File Contents: '%s' ---\n\n", dirArr[dirIndex].fileTitle[fileIndex]);
    char ch;
    while ((ch = fgetc(fptr)) != EOF) {
        putchar(ch);
    }
    printf("\n--- End of File ---\n\n");

    fclose(fptr);
    return 0;
}