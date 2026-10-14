#include <libvirt/libvirt.h>
#include <string.h>
#include <stdlib.h>

#include "cobia.h"
#include "xml.h"
#include "virt.h"

void create_domain(virConnectPtr conn, const char *name)
{
    parse_yaml(name);
    xmlBufferPtr buf = xmlBufferCreate();
    if (!buf) return;
    const char* xml = create_domain_xml(buf);
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
                virDomainUndefineFlags(doms[i], VIR_DOMAIN_UNDEFINE_KEEP_NVRAM);
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

virDomainPtr get_domain(virConnectPtr conn, const char *name)
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

char *get_domain_xml(virConnectPtr conn, const char *name)
{
    virDomainPtr dom = get_domain(conn, name);
    if (dom != NULL) {
        return virDomainGetXMLDesc(dom, 0);
    }

    return NULL;
}

void show_domains(virConnectPtr conn)
{
    virDomainPtr *doms = get_all_domains(conn);
    if (doms != NULL) {
        fprintf(stdout, "Domain List:\n");
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

virNetworkPtr *get_all_networks(virConnectPtr conn)
{
    virNetworkPtr *networks = NULL;
    TOTAL_NET_COUNT = virConnectListAllNetworks(conn, &networks, 0);
    if (TOTAL_NET_COUNT > 0) {
        return networks;
    }

    return NULL;
}

virNetworkPtr get_network(virConnectPtr conn, const char *name)
{
    virNetworkPtr *networks = get_all_networks(conn);
    if (networks != NULL) {
        for (int i = 0; i < TOTAL_NET_COUNT; ++i) {
            if (networks[i] != NULL && strcmp(virNetworkGetName(networks[i]), name) == 0)
                return networks[i];
        }
    }

    return NULL;
}

void show_networks(virConnectPtr conn)
{
    virNetworkPtr *networks = get_all_networks(conn);
    if (networks != NULL) {
        fprintf(stdout, "Network List:\n");
        for (int i = 0; i < TOTAL_NET_COUNT; ++i) {
            if (networks[i]) {
                fprintf(stdout, "  - %s\n", virNetworkGetName(networks[i]));
            }
        }
    }
}

char *get_network_xml(virConnectPtr conn, const char *name)
{
    virNetworkPtr net = get_network(conn, name);
    if (net != NULL) {
        return virNetworkGetXMLDesc(net, 0);
    }

    return NULL;
}

char *show_network_info(virConnectPtr conn, const char *name)
{
    char *netxml = get_network_xml(conn, name);
    if (netxml != NULL) {
        xmlDocPtr doc = xmlReadMemory(netxml, strlen(netxml), "n.xml", NULL, XML_PARSE_RECOVER | XML_PARSE_NOBLANKS);
        if (doc == NULL) {
            fprintf(stderr, "Failed to parse network xml\n");
            return NULL;
        }

        fprintf(stdout, "Network Info:\n");
        fprintf(stdout, "  Name: %s\n  UUID: %s\n  Bridge Name: %s\n  Mode: %s\n  IP: %s\n  DHCP Range: %s - %s\n",
                xpath_get(doc, "/network/name"),
                xpath_get(doc, "/network/uuid"),
                xpath_get(doc, "/network/bridge/@name"),
                xpath_get(doc, "/network/forward/@mode"),
                xpath_get(doc, "/network/ip/@address"),
                xpath_get(doc, "/network/ip/dhcp/range/@start"),
                xpath_get(doc, "/network/ip/dhcp/range/@end"));
    }

    return NULL;
}
