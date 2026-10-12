#include <stdio.h>
#include <string.h>
#include <yaml.h>

#include "cobia.h"
#include "xml.h"
#include "virt.h"

int TOTAL_VM_COUNT = 0;
int TOTAL_NET_COUNT = 0;

char *NAME      = NULL;
char *ARCH      = NULL;
char *BOOT      = NULL;
char *CPU       = NULL;
char *CD        = NULL;
char *DISK      = NULL;
char *MEMORY    = NULL;
char *NET       = NULL;
char *MODEL     = NULL;
char *GRAPHICS  = NULL;
char *SSHKEY    = NULL;
char *USER      = NULL;
char *VMDIR     = NULL;
char NVRAM[256] = { 0 };

typedef struct {
    const char *key;
    char **dest;
} key_map_t;

key_map_t settings[] = {
    { "arch",     &ARCH     },
    { "boot",     &BOOT     },
    { "cd",       &CD       },
    { "cpu",      &CPU      },
    { "disk",     &DISK     },
    { "graphics", &GRAPHICS },
    { "model",    &MODEL    },
    { "name",     &NAME     },
    { "net",      &NET      },
    { "ram",      &MEMORY   },
    { "ssh_key",  &SSHKEY   },
    { "user",     &USER     },
    { "vm_dir",   &VMDIR    },
};

void parse_yaml(const char *filepath)
{
    int mapping, key_count = 0;
    char current_key[256] = {0};
    char keys[256][256], values[256][256];

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

        if (event.type == YAML_MAPPING_START_EVENT)
            mapping = 1;

        if (event.type == YAML_MAPPING_END_EVENT)
            mapping = 0;

        if (event.type == YAML_SCALAR_EVENT) {
            char *value = (char *)event.data.scalar.value;
            int len = event.data.scalar.length;
            if (mapping && current_key[0] == '\0') {
                if (strcmp(value, "domain") != 0) {
                    snprintf(current_key, sizeof(current_key), "%.*s", len, value);
                    snprintf(keys[key_count], sizeof(current_key), "%.*s", len, value);
                    ++key_count;
                }
            } else {
                snprintf(values[key_count-1], sizeof(current_key), "%.*s", len, value);
                current_key[0] = '\0';
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
    }
}

int main(int argc, char **argv) {
    virConnectPtr conn;

    if (argc < 2) {
        fprintf(stderr, "Please provide argument.\n");
        return 1;
    }

    conn = virConnectOpen("qemu:///system");
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to 'qemu:///system'.\n");
        return 1;
    }

    int e = argHandle(conn, argc, argv);
    if (e != 0) return 1;

    virConnectClose(conn);
    return 0;
}

int argHandle(virConnectPtr conn, int argc, char* argv[])
{
    int opt;
    const char * arg_str = "hvc:l:s:q:D:i:p:";
    while ((opt = getopt_long(argc, argv, arg_str, options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                // TODO
            case 'v':
                // TODO
            case 'c':
                create_domain(conn, optarg);
                break;
            case 'l':
                if (strcmp(optarg, "domains") == 0) {
                    show_domains(conn);
                }
                if (strcmp(optarg, "networks") == 0) {
                    show_networks(conn);
                }
                return 0;
            case 'i':
                fprintf(stdout, "%s:\n", optarg);
                show_vm_info(optarg, conn);
                return 0;
            case 's':
                start_domain(conn, optarg);
                return 0;
            case 'q':
                stop_domain(conn, optarg);
                return 0;
            case 'D':
                get_nvram_path(optarg);
                fprintf(stdout, "%s\n", NVRAM);
                delete_domain(conn, optarg);
                break;
            case '?':
                default:
                break;
        }
    }

    return 0;
}
