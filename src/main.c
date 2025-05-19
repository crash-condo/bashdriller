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

#define INPUT_BUF 512
#define MAX_ENTRIES 256
#define DRILL_PATH "/etc/bashdriller/"
#define DRILL_EXT ".drill"

#define CONTINUE_DRILL 0
#define SKIP_TO_NEXT   1
#define RETURN_TO_MENU 2

typedef struct {
    char description[INPUT_BUF];
    char command[INPUT_BUF];
    char explanation[INPUT_BUF];
    char reference[INPUT_BUF];
    char tags[INPUT_BUF];
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
    char line[INPUT_BUF];
    DrillItem current = {0};
    int fields_set = 0;

    while (fgets(line, sizeof(line), fp)) {
        // New entry begins
        if (line[0] == '(' && isdigit(line[1])) {
            if (fields_set == 5 && index >= 0) {
                set->items[index] = current;
            } else if (index >= 0) {
                log_error(filepath, index + 1);
            }
            memset(&current, 0, sizeof(DrillItem));
            fields_set = 0;
            index++;
            if (index >= MAX_ENTRIES) break;
            continue;
        }

        // Skip non-kv lines
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';

        char *key = line;
        char *val = eq + 1;

        // Trim whitespace
        while (*val == ' ' || *val == '\t') val++;
        size_t len = strlen(val);
        while (len > 0 && (val[len - 1] == '\n' || val[len - 1] == '\r')) val[--len] = '\0';

        if (val[0] != '"' || val[len - 1] != '"') {
            log_error(filepath, index + 1);
            continue;
        }

        val[len - 1] = '\0';
        val = val + 1;  // skip leading quote

        if (strcmp(key, "desc") == 0) {
            strncpy(current.description, val, INPUT_BUF - 1);
            fields_set++;
        } else if (strcmp(key, "exp_comm") == 0) {
            strncpy(current.command, val, INPUT_BUF - 1);
            fields_set++;
        } else if (strcmp(key, "expl") == 0) {
            strncpy(current.explanation, val, INPUT_BUF - 1);
            fields_set++;
        } else if (strcmp(key, "ref") == 0) {
            strncpy(current.reference, val, INPUT_BUF - 1);
            fields_set++;
        } else if (strcmp(key, "tags") == 0) {
            strncpy(current.tags, val, INPUT_BUF - 1);
            fields_set++;
        }
    }

    // Save final block if complete
    if (fields_set == 5 && index >= 0) {
        set->items[index] = current;
    } else if (index >= 0) {
        log_error(filepath, index + 1);
    }

    set->count = index + 1;
    fclose(fp);
    return 1;
}


int run_drill(const DrillItem *item) {
    int correct = 0;
    char input[INPUT_BUF];

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

