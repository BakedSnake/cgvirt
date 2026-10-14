#include <libvirt/libvirt.h>

void create_domain(virConnectPtr conn, const char *name);

void delete_domain(virConnectPtr conn, const char *name);

void start_domain(virConnectPtr conn, const char *name);

void stop_domain(virConnectPtr conn, const char *name);

virDomainPtr *get_all_domains(virConnectPtr conn);

virDomainPtr get_domain(virConnectPtr conn, const char *name);

char *get_domain_xml(virConnectPtr conn, const char *name);

void show_domains(virConnectPtr conn);

void show_vm_info(char *name, virConnectPtr conn);

virNetworkPtr *get_all_networks(virConnectPtr conn);

virNetworkPtr get_network(virConnectPtr conn, const char *name);

void show_networks(virConnectPtr conn);

char *get_network_xml(virConnectPtr conn, const char *name);
