// BashDriller – A terminal-native tool for repetitive Bash command training and retention
// Copyright (C) 2025 Michael C. Condon
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//
// Contact: mcc_gh@abine.org

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <errno.h>

#define MAX_INPUT 512
#define MAX_ENTRIES 256
#define DRILL_PATH "/etc/bashdriller/"
#define DRILL_EXT ".drill"

#define CONTINUE_DRILL 0
#define SKIP_TO_NEXT   1
#define RETURN_TO_MENU 2

typedef struct {
    char description[MAX_INPUT];
    char command[MAX_INPUT];
    char explanation[MAX_INPUT];
    char reference[MAX_INPUT];
    char tags[MAX_INPUT];
} DrillItem;

typedef struct {
    char name[64];
    DrillItem items[MAX_ENTRIES];
    int count;
} DrillSet;

int compare_names(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void log_error(const char *filename, int entry_number) {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "logger -p user.err \"bashdriller: malformed entry in %s (entry %d)\"", filename, entry_number);
    system(cmd);
}

int parse_drill_file(const char *filepath, DrillSet *set) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) return 0;

    int index = -1;
    char line[MAX_INPUT];
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '(' && isdigit(line[1])) {
            index++;
            if (index >= MAX_ENTRIES) break;
            continue;
        }
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "\n");

        if (!key || !value || key[0] != '"' || value[0] != ' ') {
            log_error(filepath, index + 1);
            continue;
        }

        value++; // skip space before opening quote
        if (strncmp(key, "\"description\"", 13) == 0)
            strncpy(set->items[index].description, value + 1, MAX_INPUT - 1);
        else if (strncmp(key, "\"expected command\"", 18) == 0)
            strncpy(set->items[index].command, value + 1, MAX_INPUT - 1);
        else if (strncmp(key, "\"explanation\"", 13) == 0)
            strncpy(set->items[index].explanation, value + 1, MAX_INPUT - 1);
        else if (strncmp(key, "\"reference\"", 11) == 0)
            strncpy(set->items[index].reference, value + 1, MAX_INPUT - 1);
        else if (strncmp(key, "\"tags\"", 6) == 0)
            strncpy(set->items[index].tags, value + 1, MAX_INPUT - 1);
    }
    fclose(fp);
    set->count = index + 1;
    return 1;
}

int run_drill(const DrillItem *item) {
    int correct = 0;
    char input[MAX_INPUT];

    while (correct < 5) {
        printf("%s\n> ", item->description);
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "showit") == 0) {
            printf("Expected command: %s\n", item->command);
            continue;
        }
        if (strcmp(input, "expit") == 0) {
            printf("Explanation: %s\n", item->explanation);
            continue;
        }
        if (strcmp(input, "next") == 0) {
            printf("Skipping to next drill.\n");
            return SKIP_TO_NEXT;
        }
        if (strcmp(input, "quit") == 0) {
            printf("Returning to main menu.\n");
            return RETURN_TO_MENU;
        }
        if (strcmp(input, item->command) == 0) {
            correct++;
            printf("Correct (%d/5)\n", correct);
        } else {
            printf("Incorrect. Try again.\n");
        }
    }
    return CONTINUE_DRILL;
}

int main() {
    DIR *dir = opendir(DRILL_PATH);
    if (!dir) {
        perror("Failed to open /etc/bashdriller/");
        return 1;
    }

    struct dirent *entry;
    char *filenames[MAX_ENTRIES];
    int file_count = 0;

    while ((entry = readdir(dir)) && file_count < MAX_ENTRIES) {
        if (strstr(entry->d_name, DRILL_EXT)) {
            filenames[file_count] = strdup(entry->d_name);
            file_count++;
        }
    }
    closedir(dir);

    qsort(filenames, file_count, sizeof(char *), compare_names);

    while (1) {
        printf("\nDrills Available:\n");
        for (int i = 0; i < file_count; i++) {
            filenames[i][strlen(filenames[i]) - strlen(DRILL_EXT)] = '\0';
            printf("%d) %s\n", i + 1, filenames[i]);
        }
        printf("0) Quit\n");

        printf("Select a drill by number: ");
        int choice = 0;
        scanf("%d", &choice);
        getchar();

        if (choice == 0) break;
        if (choice < 1 || choice > file_count) {
            printf("Invalid selection.\n");
            continue;
        }

        char fullpath[256];
        snprintf(fullpath, sizeof(fullpath), "%s%s.drill", DRILL_PATH, filenames[choice - 1]);
        filenames[choice - 1][strlen(filenames[choice - 1])] = '.'; // restore ".drill" for reuse

        DrillSet current = {0};
        strncpy(current.name, filenames[choice - 1], sizeof(current.name) - 1);
        if (!parse_drill_file(fullpath, &current)) {
            printf("Failed to load drill file.\n");
            continue;
        }

        for (int i = 0; i < current.count; i++) {
            int result = run_drill(&current.items[i]);
            if (result == RETURN_TO_MENU) break;
        }
    }

    for (int i = 0; i < file_count; i++) free(filenames[i]);
    return 0;
}

