#include <stdio.h>
#include <string.h>
#include <yaml.h>

#include "cobia.h"
#include "xml.h"
#include "virt.h"

#define KVSIZE 256
#define MAXDEPTH 8
#define MAXKEYS 256

int TOTAL_VM_COUNT      = 0;
int TOTAL_NET_COUNT     = 0;

char *NAME              = NULL;
char *ARCH              = NULL;
char *BOOT              = NULL;
char *BOOTDEV           = NULL;
char *CPU               = NULL;
char *CD                = NULL;
char *DISK              = NULL;
char *DISKSIZE          = NULL;
char *EFI               = NULL;
char *MACHINE           = NULL;
char *MEMORY            = NULL;
char *NET               = NULL;
char *MODEL             = NULL;
char *GRAPHICS          = NULL;
char *SECBOOT           = NULL;
char *SSHKEY            = NULL;
char *USER              = NULL;
char *VIDEO_DRV         = NULL;
char *VIDEO_MDL         = NULL;
char *VMDIR             = NULL;
char NVRAM[256]         = { 0 };

key_map_t settings[]    = {
    { "domain.boot",                        &BOOT     },
    { "domain.cpu",                         &CPU      },
    { "domain.devices.cd",                  &CD       },
    { "domain.devices.disk.size",           &DISKSIZE },
    { "domain.devices.disk.source",         &DISK     },
    { "domain.devices.graphics",            &GRAPHICS },
    { "domain.devices.net",                 &NET      },
    { "domain.devices.video.driver",        &VIDEO_DRV},
    { "domain.devices.video.model",         &VIDEO_MDL},
    { "domain.model",                       &MODEL    },
    { "domain.name",                        &NAME     },
    { "domain.os.arch",                     &ARCH     },
    { "domain.os.bootdev",                  &BOOTDEV  },
    { "domain.os.efi",                      &EFI      },
    { "domain.os.features.secure_boot",     &SECBOOT  },
    { "domain.os.machine",                  &MACHINE  },
    { "domain.ram",                         &MEMORY   },
    { "domain.ssh_key",                     &SSHKEY   },
    { "domain.user",                        &USER     },
    { "domain.vm_dir",                      &VMDIR    },
};

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
    const char * arg_str = "hvc:l:s:q:D:i:p:x:";
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
            case 'x':
                fprintf(stdout, "%s\n", get_domain_xml(conn, optarg));
                return 0;
            case '?':
                default:
                break;
        }
    }

    return 0;
}

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
