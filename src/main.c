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
#include "cJSON.h"

#define MAX_INPUT 512
#define MAX_DRILLS 64

void run_drill(const char *description, const char *expected_command) {
    char input[MAX_INPUT];
    int correct_count = 0;

    while (correct_count < 5) {
        printf("%s\n> ", description);
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, expected_command) == 0) {
            correct_count++;
            printf("Correct (%d/5)\n", correct_count);
        } else {
            printf("Incorrect. Try again.\n");
        }
    }
}

int main() {
    FILE *file = fopen("commands.json", "r");
    if (!file) {
        perror("Failed to open commands.json");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char *data = malloc(length + 1);
    fread(data, 1, length, file);
    data[length] = '\0';
    fclose(file);

    cJSON *root = cJSON_Parse(data);
    if (!root) {
        fprintf(stderr, "Failed to parse JSON\n");
        free(data);
        return 1;
    }

    int total_categories = cJSON_GetArraySize(root);
    cJSON *categories[MAX_DRILLS];
    const char *category_names[MAX_DRILLS];
    int category_count = 0;

    cJSON *entry = NULL;
    cJSON_ArrayForEach(entry, root) {
        if (category_count >= MAX_DRILLS) break;
        category_names[category_count] = entry->string;
        categories[category_count] = entry;
        category_count++;
    }

    printf("Drills Available:\n");
    for (int i = 0; i < category_count; i++) {
        printf("%d) %s\n", i + 1, category_names[i]);
    }

    printf("Select a drill by number: ");
    int selection = 0;
    scanf("%d", &selection);
    getchar(); // flush newline

    if (selection < 1 || selection > category_count) {
        printf("Invalid selection.\n");
        cJSON_Delete(root);
        free(data);
        return 1;
    }

    cJSON *drill_array = categories[selection - 1];
    int drill_count = cJSON_GetArraySize(drill_array);
    for (int i = 0; i < drill_count; i++) {
        cJSON *item = cJSON_GetArrayItem(drill_array, i);
        const char *desc = cJSON_GetObjectItem(item, "description")->valuestring;
        const char *cmd = cJSON_GetObjectItem(item, "command")->valuestring;
        run_drill(desc, cmd);
    }

    cJSON_Delete(root);
    free(data);
    return 0;
}

