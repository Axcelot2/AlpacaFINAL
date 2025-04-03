// Xi Ting Yan wrote this program.
// Ali Kutay Dastan wrote the program that interfaces with Ollama, was responsible for version control, and designed the repository that was used for this project.
// Tahmeed Ahmed is responsible for debugging and error handling.
// Ashkan Sharifi is responsible for implementing a feature to print contents of files into the program.
// This program takes user input to create directories and files with the specified titles and contents.
// Compile alpacaMain.c with alpacaLocal.c in the same directory as alpacaLocal.h and dirStruct.h.
// The final goal of project Alpaca is to implement data streaming from Ollama into this program to create local files that store Ollama context.
// Ollama is an open-source software that allows local running of LLMs.

#include <stdio.h>
#include <stdlib.h>
#include "alpacaLocal.h"




// The main program uses functions defined in alpacaLocal.c.
int main() {
    // whatDo stores user request for function.
    // dirArr is an array of structs, with each struct representing a directory.
    int whatDo;
    struct dirStruct dirArr[MAX_DIR];

    // Functions called later will check if dirTitle[0] is NULL as a means of verifying if it has been initialized.
    for (int i = 0; i < MAX_DIR; i++) {
        dirArr[i].dirTitle[0] = '\0';
    }

    for (int i = 0; i < MAX_DIR; i++) {
        for (int x = 0; x < MAX_FILES; x++) {
            dirArr[i].fileTitle[x][0] = '\0';
        }
    }

    // This function indexes existing directories in the same directory as the program. See alpacaLocal.c.
    // If this function fails, an error message is printed and the program terminates. (See alpacaLocal.c for error message.)
    if (indexDir(dirArr) == 1) {
        return 1;
    }

    if (indexFile(dirArr) == 1) {
        return 1;
    }

    printf("\n--- Alpaca: Ollama Assistant ---\n\n");

    do {
        // Menu options displyed.
        printf("Menu Options: \n");
        printf("1. List Available Models\n");
        printf("2. Create Context Category\n");
        printf("3. List Categories\n");
        printf("4. List Context Files\n");
        printf("5. Print Context File\n");
        printf("6. Chat with Model\n");
        printf("0. Exit Alpaca\n\n");
        printf("Enter your selection: ");
        scanf("%d", &whatDo);
        getchar(); // Buffer overflow protection for fgets() in called functions.
        printf("\n");

        switch (whatDo) {
            case 1:
                // This function creates a directory, see alpacaLocal.c for definition and error messages.
                if (system("ollama list") != 0) {
                    printf("No models found.\n\n");
                } else {
                    printf("End of list of models.\n\n");
                }
                break;

            case 2:
                // This function creates a file, see alpacaLocal.c for definition and error messages.
                createDir(dirArr);
                break;

            case 3:
                // This function prints all indexed directories, see alpacaLocal.c for definition.
                // Function return 0 after successful execution. Returns 1 if no indexed directories exist.
                listDir(dirArr);
                break;

            case 4:
                int dirSelect = 0;
                if (listDir(dirArr) == 0) {
                    printf("Select a directory: ");
                    scanf("%d", &dirSelect);
                    dirSelect--;
                    if (dirSelect < 0 || dirSelect >= MAX_DIR || dirArr[dirSelect].dirTitle[0] == '\0') {
                        printf("Invalid Category Entered.\n\n");
                    } else {
                        listFile(dirArr, dirSelect);
                    }
                }
                break;

            case 5:
                printFileContent(dirArr);
                break;
                
            // case 6:
            //     if (system("ollama list") != 0) {
            //         printf("No models found.\n");
            //         break;
            //     } else {
            //         char modelName[MAX_MODEL_NAME];
            //         printf("Type in the selected model name: ");
            //         fgets(modelName, sizeof(modelName), stdin);
            //         if (chatWithOllama(modelName, dirArr) == 1) {
            //             break;
            //         }
            //     }
            //     break;

            case 0:
                printf("Exiting Program.\n\n");
                break;
            
            default:
                // Input validation.
                printf("Please enter a valid input according to the menu options.\n\n");
                break;
        }

    // Program terminates at user request.
    } while (whatDo != 0);

    return 0;
}