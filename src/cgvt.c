#include <stdio.h>
#include <string.h>

#include "cgvt.h"

int TOTAL_VM_COUNT = 0;

int main(int argc, char **argv) {
    virConnectPtr conn;

    if (argc < 2) {
        fprintf(stderr, "Please provide vm name.\n");
        return 1;
    }

    conn = virConnectOpen("qemu:///system");
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to 'qemu:///system'.\n");
        return 1;
    }

    fprintf(stdout, "List:\n");
    list_vms(conn);
    fprintf(stdout, "\n");

    fprintf(stdout, "%s:\n", argv[1]);
    show_vm_info(argv[1], conn);

    virDomainPtr dom = get_domain(conn, argv[1]);
    if (dom != NULL) {
        const char *name = virDomainGetName(dom);
        fprintf(stdout, "\nGot Domain: %s\n", name);
    }

    virConnectClose(conn);
    return 0;
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

void list_vms(virConnectPtr conn)
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
                    fprintf(stdout, "  State: %s, Max Memory: %lu, CPU nr: %d\n",
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
