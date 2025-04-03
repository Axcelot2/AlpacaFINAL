#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <windows.h>
#include <direct.h>

#define RESPONSE_BUFFER_SIZE 8192
#define PROMPT_BUFFER_SIZE 2048

char conversation_log[RESPONSE_BUFFER_SIZE];
char log_filename[260] = "conversation_log.txt";

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total_size = size * nmemb;
    char *chunk = (char *)ptr;
    chunk[total_size] = '\0';

    char *start = strstr(chunk, "\"response\":\"");
    if (start) {
        start += strlen("\"response\":\"");
        char *end = strchr(start, '"');
        if (end) {
            *end = '\0';
            for (char *p = start; *p; ++p) {
                if (p[0] == '\\' && p[1] == 'n') {
                    p[0] = '\n';
                    memmove(p + 1, p + 2, strlen(p + 2) + 1);
                }
            }
            printf("%s", start);
            strncat(conversation_log, start, RESPONSE_BUFFER_SIZE - strlen(conversation_log) - 1);
            *end = '"';
        }
    }

    return total_size;
}

void send_prompt_to_ollama(const char *prompt) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        printf("Failed to initialize libcurl.\n");
        return;
    }

    char json_payload[PROMPT_BUFFER_SIZE * 2];
    snprintf(json_payload, sizeof(json_payload),
             "{ \"model\": \"mistral\", \"prompt\": \"%s\", \"stream\": false }",
             prompt);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/generate");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

void save_conversation(const char *prompt) {
    FILE *log = fopen(log_filename, "a");
    if (log) {
        fprintf(log, "User: %s\n", prompt);
        fprintf(log, "LLM: %s\n\n", conversation_log);
        fclose(log);
    } else {
        printf("Failed to write conversation log.\n");
    }
    conversation_log[0] = '\0';
}

void list_files_and_dirs(char items[][260], int *count, int *is_dir_flags) {
    WIN32_FIND_DATAA fileData;
    HANDLE hFind = FindFirstFileA("*", &fileData);
    *count = 0;

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(fileData.cFileName, ".") != 0 && strcmp(fileData.cFileName, "..") != 0) {
                strcpy(items[*count], fileData.cFileName);
                is_dir_flags[*count] = (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
                (*count)++;
            }
        } while (FindNextFileA(hFind, &fileData));
        FindClose(hFind);
    }
}

void escape_json_string(const char *input, char *output, size_t max_len) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\0' && j < max_len - 1; i++) {
        if (input[i] == '"') {
            if (j + 2 >= max_len) break;
            output[j++] = '\\';
            output[j++] = '"';
        } else if (input[i] == '\\') {
            if (j + 2 >= max_len) break;
            output[j++] = '\\';
            output[j++] = '\\';
        } else if (input[i] == '\n') {
            if (j + 2 >= max_len) break;
            output[j++] = '\\';
            output[j++] = 'n';
        } else {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}

void upload_text_file_to_ollama(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Failed to open file: %s\n", filename);
        return;
    }

    char raw_prompt[PROMPT_BUFFER_SIZE];
    size_t len = fread(raw_prompt, 1, PROMPT_BUFFER_SIZE - 1, fp);
    raw_prompt[len] = '\0';
    fclose(fp);

    char escaped_prompt[PROMPT_BUFFER_SIZE * 2];
    escape_json_string(raw_prompt, escaped_prompt, sizeof(escaped_prompt));

    conversation_log[0] = '\0';
    send_prompt_to_ollama(escaped_prompt);
    save_conversation(raw_prompt);
}

void select_log_file() {
    printf("\nEnter name of the log file to save conversation (e.g., my_log.txt): ");
    fgets(log_filename, sizeof(log_filename), stdin);
    log_filename[strcspn(log_filename, "\n")] = '\0';
    printf("Using '%s' as log file.\n", log_filename);
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);

    int choice;
    char prompt[PROMPT_BUFFER_SIZE];

    while (1) {
        printf("\nMenu:\n");
        printf("1. Chat with model\n");
        printf("2. Browse and select file/folder\n");
        printf("3. Choose conversation log file\n");
        printf("0. Quit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();

        if (choice == 0) {
            printf("Exiting...\n");
            break;
        } else if (choice == 1) {
            printf("Enter your question: ");
            fgets(prompt, sizeof(prompt), stdin);
            prompt[strcspn(prompt, "\n")] = '\0';
            conversation_log[0] = '\0';
            send_prompt_to_ollama(prompt);
            save_conversation(prompt);
        } else if (choice == 2) {
            char items[200][260];
            int is_dir[200];
            int item_count;

            while (1) {
                list_files_and_dirs(items, &item_count, is_dir);
                printf("\nItems in current directory:\n");
                printf("0. Go back\n");
                for (int i = 0; i < item_count; i++) {
                    printf("%d. %s%s\n", i + 1, items[i], is_dir[i] ? "/" : "");
                }

                printf("Select a file or folder: ");
                int selection;
                scanf("%d", &selection);
                getchar();

                if (selection == 0) break;
                if (selection < 1 || selection > item_count) {
                    printf("Invalid selection.\n");
                    continue;
                }

                if (is_dir[selection - 1]) {
                    _chdir(items[selection - 1]);
                } else {
                    upload_text_file_to_ollama(items[selection - 1]);
                    break;
                }
            }
        } else if (choice == 3) {
            select_log_file();
        } else {
            printf("Invalid choice.\n");
        }
    }

    curl_global_cleanup();
    return 0;
}
