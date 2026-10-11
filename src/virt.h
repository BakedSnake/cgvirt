#include <libvirt/libvirt.h>

void create_domain(virConnectPtr conn);

void delete_domain(virConnectPtr conn, const char *name);

void start_domain(virConnectPtr conn, const char *name);

void stop_domain(virConnectPtr conn, const char *name);

virDomainPtr *get_all_domains(virConnectPtr conn);

virDomainPtr get_domain(virConnectPtr conn, char *name);

void show_domains(virConnectPtr conn);

void show_vm_info(char *name, virConnectPtr conn);

virNetworkPtr *get_all_networks(virConnectPtr conn);

void show_networks(virConnectPtr conn);

