#include <stdio.h>
#include <string.h>

#include "cobia.h"
#include "xml.h"
#include "virt.h"

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

key_map_t settings[20]  = {
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
    const char * arg_str = "hvc:l:s:q:D:i:p:x:n:";
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
                break;
            case 'i':
                fprintf(stdout, "%s:\n", optarg);
                show_vm_info(optarg, conn);
                break;
            case 's':
                start_domain(conn, optarg);
                break;
            case 'q':
                stop_domain(conn, optarg);
                break;
            case 'D':
                get_nvram_path(optarg);
                fprintf(stdout, "%s\n", NVRAM);
                delete_domain(conn, optarg);
                break;
            case 'x':
                fprintf(stdout, "%s\n", get_domain_xml(conn, optarg));
                break;
            case 'n':
                show_network_info(conn, optarg);
                break;
            case '?':
                default:
                break;
        }
    }

    return 0;
}
