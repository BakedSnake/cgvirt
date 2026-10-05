#include <stdio.h>
#include <string.h>

#include "cgvt.h"

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

    list_vms(conn);

    fprintf(stdout, "%s:\n", argv[1]);
    get_vm_info(argv[1], conn);

    virConnectClose(conn);
    return 0;
}

void list_vms(virConnectPtr conn)
{
    int domains;
    virDomainPtr dom;

    domains = virConnectNumOfDomains(conn);
    fprintf(stdout, "Defined active domains: %d\n", domains);

    int ids[domains];
    int n = virConnectListDomains(conn, ids, domains);
    for (int i = 0; i < n; ++i) {
        dom = virDomainLookupByID(conn, ids[i]);
        if (dom) {
            fprintf(stdout, "  %s :: %d\n", virDomainGetName(dom), ids[i]);
            virDomainFree(dom);
        }
    }
    fprintf(stdout, "\n");

    domains = virConnectNumOfDefinedDomains(conn);
    fprintf(stdout, "Defined domains: %d\n", domains);

    char *names[domains];
    n = virConnectListDefinedDomains(conn, names, domains);
    for (int i = 0; i < n; ++i) {
        dom = virDomainLookupByName(conn, names[i]);
        if (dom) {
            fprintf(stdout, "  %s :: %d\n", names[i], virDomainGetID(dom));
            virDomainFree(dom);
        }
    }
    fprintf(stdout, "\n");
}

void get_vm_info(char *name, virConnectPtr conn)
{
    int domains = virConnectNumOfDefinedDomains(conn);
    if (domains > 0) {
        char *names[domains];
        int n = virConnectListDefinedDomains(conn, names, domains);
        for (int i = 0; i < n; ++i) {
            virDomainPtr dom = virDomainLookupByName(conn, names[i]);
            if (dom && strcmp(virDomainGetName(dom), name) == 0) {
                virDomainInfo dm;
                if (virDomainGetInfo(dom, &dm) == 0) {
                    fprintf(stdout, "State: %s, Max Memory: %lu, CPU nr: %d\n",
                            dm.state == 1 ? "Running" : "Stopped", dm.maxMem, dm.nrVirtCpu);
                }
            }

            virDomainFree(dom);
        }
    }

    domains = virConnectNumOfDomains(conn);
    if (domains > 0) {
        int ids[domains];
        int n = virConnectListDomains(conn, ids, domains);
        for (int i = 0; i < n; ++i) {
            virDomainPtr dom = virDomainLookupByID(conn, ids[i]);
            if (dom && strcmp(virDomainGetName(dom), name) == 0) {
                virDomainInfo dm;
                if (virDomainGetInfo(dom, &dm) == 0) {
                    fprintf(stdout, "State: %s, Max Memory: %lu, CPU nr: %d\n",
                            dm.state == 1 ? "Running" : "Stopped", dm.maxMem, dm.nrVirtCpu);
                }
            }

            virDomainInterfacePtr *ifaces = NULL;
            int nc = virDomainInterfaceAddresses(dom, &ifaces, VIR_DOMAIN_INTERFACE_ADDRESSES_SRC_LEASE, 0);
            for (int i = 0; i < nc; ++i) {
                for (size_t j = 0; j < ifaces[i]->naddrs; ++j) {
                    fprintf(stdout, "IP Address: %s\n", ifaces[i]->addrs[j].addr);
                }
            }

            virDomainFree(dom);
        }
    }
}
