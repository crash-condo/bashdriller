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

void run_drill(const char *description, const char *expected_command) {
    char input[MAX_INPUT];
    int correct_count = 0;

    while (correct_count < 10) {
        printf("%s\n> ", description);
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, expected_command) == 0) {
            correct_count++;
            printf("Correct (%d/10)\n", correct_count);
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

    int count = cJSON_GetArraySize(root);
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(root, i);
        const char *desc = cJSON_GetObjectItem(item, "description")->valuestring;
        const char *cmd = cJSON_GetObjectItem(item, "command")->valuestring;
        run_drill(desc, cmd);
    }

    cJSON_Delete(root);
    free(data);
    return 0;
}

