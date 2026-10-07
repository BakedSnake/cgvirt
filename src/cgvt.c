#include <stdio.h>
#include <string.h>
#include <libxml/xmlwriter.h>

#include "cgvt.h"
#include "xml.h"

int TOTAL_VM_COUNT = 0;

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

int argHandle(virConnectPtr conn, int argc, char* argv[])
{
    int opt;
    const char * arg_str = "hvli:a:b:c:d:g:r:m:n:w:o:s:q:O:x";
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
                fprintf(stdout, "List:\n");
                show_domains(conn);
                fprintf(stdout, "\n");
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
            case '?':
                default:
                break;
        }
    }

    return 0;
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
    if (e != 0) return 0;

    virConnectClose(conn);
    return 0;
}

void create_domain(virConnectPtr conn)
{
    xmlBufferPtr buf = xmlBufferCreate();
    if (!buf) return;
    const char* xml = get_domain_xml(buf);
    printf("%s\n", xml);
    virDomainDefineXML(conn, xml);
    xmlBufferFree(buf);
}

void delete_domain(virConnectPtr conn, const char *name)
{
    virDomainPtr *doms = get_all_domains(conn);
    if (doms != NULL) {
        for (int i = 0; i < TOTAL_VM_COUNT; ++i) {
            const char *dom_name = virDomainGetName(doms[i]);
            if (doms[i] != NULL && strcmp(dom_name, name) == 0)
                virDomainUndefine(doms[i]);
        }
    }
}

void start_domain(virConnectPtr conn, const char *name)
{
    virDomainPtr *doms = get_all_domains(conn);
    if (doms != NULL) {
        for (int i = 0; i < TOTAL_VM_COUNT; ++i) {
            const char *dom_name = virDomainGetName(doms[i]);
            if (doms[i] != NULL && strcmp(dom_name, name) == 0) {
                virDomainCreate(doms[i]);
            }
        }
    }
}

void stop_domain(virConnectPtr conn, const char *name)
{
    virDomainPtr *doms = get_all_domains(conn);
    if (doms != NULL) {
        for (int i = 0; i < TOTAL_VM_COUNT; ++i) {
            const char *dom_name = virDomainGetName(doms[i]);
            if (doms[i] != NULL && strcmp(dom_name, name) == 0) {
                virDomainDestroy(doms[i]);
            }
        }
    }
}

virDomainPtr *get_all_domains(virConnectPtr conn)
{
    virDomainPtr *domains = NULL;
    TOTAL_VM_COUNT = virConnectListAllDomains(conn, &domains, 0);
    if (TOTAL_VM_COUNT > 0) {
        return domains;
    }

    return NULL;
}

virDomainPtr get_domain(virConnectPtr conn, char *name)
{
    virDomainPtr *doms = get_all_domains(conn);
    if (doms != NULL) {
        for (int i = 0; i < TOTAL_VM_COUNT; ++i) {
            if (doms[i] != NULL && strcmp(virDomainGetName(doms[i]), name) == 0) {
                return doms[i];
            }
        }
    }

    return NULL;
}

void show_domains(virConnectPtr conn)
{
    virDomainPtr *doms = get_all_domains(conn);
    if (doms != NULL) {
        for (int i = 0; i < TOTAL_VM_COUNT; ++i) {
            if (doms[i]) {
                const char *name = virDomainGetName(doms[i]);
                int id = virDomainGetID(doms[i]);
                fprintf(stdout, "  %d - %s\n", id, name);
            }

            virDomainFree(doms[i]);
        }
    }
}

void show_vm_info(char *name, virConnectPtr conn)
{
    virDomainPtr *doms = get_all_domains(conn);
    if (doms != NULL) {
        for (int i = 0; i < TOTAL_VM_COUNT; ++i) {
            virDomainPtr dom = doms[i];
            if (dom != NULL && strcmp(virDomainGetName(dom), name) == 0) {
                virDomainInfo dm;
                if (virDomainGetInfo(dom, &dm) == 0) {
                    fprintf(stdout, "  State: %s\n  Max Memory: %lu\n  CPU nr: %d\n",
                            dm.state == 1 ? "Running" : "Stopped", dm.maxMem, dm.nrVirtCpu);
                }

                if (dm.state == 1) {
                    virDomainInterfacePtr *ifaces = NULL;
                    int nc = virDomainInterfaceAddresses(dom, &ifaces, VIR_DOMAIN_INTERFACE_ADDRESSES_SRC_LEASE, 0);
                    for (int i = 0; i < nc; ++i) {
                        for (size_t j = 0; j < ifaces[i]->naddrs; ++j) {
                            fprintf(stdout, "  IP Address: %s\n", ifaces[i]->addrs[j].addr);
                        }
                    }
                }
            }


            virDomainFree(dom);
        }
    }
}
