#include <stdio.h>
#include <string.h>

#include "cobia.h"
#include "xml.h"
#include "virt.h"

int TOTAL_VM_COUNT = 0;
int TOTAL_NET_COUNT = 0;

char *NAME = NULL;
char *ARCH = NULL;
char *BOOT = NULL;
char *CPU  = NULL;
char *CD   = NULL;
char *MEMORY  = NULL;
char *DISK = NULL;
char *NET = NULL;
char *MODEL = NULL;
char *GRAPHICS = NULL;
char NVRAM[256] = { 0 };


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
    const char * arg_str = "hvl:i:a:b:c:d:g:r:m:n:w:o:s:q:O:X:x";
    while ((opt = getopt_long(argc, argv, arg_str, options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                // TODO
            case 'v':
                // TODO
            case 'a':
                ARCH = optarg;
                break;
            case 'b':
                BOOT = optarg;
                break;
            case 'c':
                CPU = optarg;
                break;
            case 'd':
                DISK = optarg;
                break;
            case 'w':
                NET = optarg;
                break;
            case 'm':
                MODEL = optarg;
                break;
            case 'g':
                GRAPHICS = optarg;
                break;
            case 'n':
                NAME = optarg;
                break;
            case 'r':
                MEMORY = optarg;
                break;
            case 'O':
                CD = optarg;
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
            case 'x':
                create_domain(conn);
                break;
            case 'X':
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
