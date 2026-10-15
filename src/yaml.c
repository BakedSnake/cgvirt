#include "yaml.h"

#include "cobia.h"

void parse_yaml(const char *filepath)
{
    int depth = 0;
    int key_count = 0;
    char keybuf[MAXDEPTH][KVSIZE] = {0};
    char keys[MAXKEYS][KVSIZE] = {0};
    char values[MAXKEYS][KVSIZE] = {0};

    FILE *fp = fopen(filepath, "rb");
    yaml_parser_t parser;
    yaml_event_t event;

    if (!yaml_parser_initialize(&parser))
        fprintf(stderr, "Failed to init yaml parser.\n");

    if (fp == NULL)
        fprintf(stderr, "Failed to open file\n");

    yaml_parser_set_input_file(&parser, fp);
    while(1) {
        if (!yaml_parser_parse(&parser, &event))
            break;

        if (event.type == YAML_MAPPING_START_EVENT) {
            ++depth;
        }

        if (event.type == YAML_MAPPING_END_EVENT) {
            --depth;
            if (depth > 0) keybuf[depth - 1][0] = '\0';
        }

        if (event.type == YAML_SCALAR_EVENT) {
            char *value = (char *)event.data.scalar.value;
            int len = event.data.scalar.length;

            if (depth > 0  && keybuf[depth - 1][0] == '\0') {
                snprintf(keybuf[depth - 1], KVSIZE, "%.*s", len, value);

                char path[KVSIZE * MAXDEPTH] = {0};
                for (int d = 0; d < depth; ++d) {
                    if (d > 0) strncat(path, ".", sizeof(path) - strlen(path) - 1);
                    strncat(path, keybuf[d], sizeof(path) - strlen(path) - 1);
                }
                snprintf(keys[key_count], KVSIZE, "%s", path);
            } else {
                if (key_count < MAXKEYS) {
                    snprintf(values[key_count], KVSIZE, "%.*s", len, value);
                    ++key_count;
                }

                if (depth > 0)
                    keybuf[depth - 1][0] = '\0';
            }
        }

        if (event.type == YAML_STREAM_END_EVENT)
            break;

        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);
    fclose(fp);

    // Assign parsed values
    for (int i = 0; i < key_count; ++i) {
        for (size_t j = 0; j < sizeof(settings)/sizeof(settings[0]); ++j) {
            if (strcmp(keys[i], settings[j].key) == 0)
                *settings[j].dest = values[i];
        }

        fprintf(stdout, "%s: %s\n", settings[i].key, *settings[i].dest);
        //fprintf(stdout, "%s: %s\n", keys[i], values[i]);
    }
}
