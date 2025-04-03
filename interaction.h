#include <windows.h>
#include "alpacaLocal.h"
#include <curl/curl.h>
#include <direct.h>

#define RESPONSE_BUFFER_SIZE 8192
#define PROMPT_BUFFER_SIZE 2048
#define MAX_MODEL_NAME 25

char conversation_log[RESPONSE_BUFFER_SIZE];
char log_filename[260] = "conversation_log.txt";

// Interaction.c functions integrated fully
size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata);

void send_prompt_to_ollama(const char *prompt);

void save_conversation(const char *prompt);

void list_files_and_dirs(char items[][260], int *count, int *is_dir_flags);

void escape_json_string(const char *input, char *output, size_t max_len);
  


void upload_text_file_to_ollama(const char *filename);
    

void select_log_file();

int chatWithOllama(char modelName[MAX_MODEL_NAME], struct dirStruct dirArr[MAX_DIR]);
