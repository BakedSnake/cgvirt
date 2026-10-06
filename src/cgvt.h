#include <libvirt/libvirt.h>

virDomainPtr *get_all_domains(virConnectPtr conn);

virDomainPtr get_domain(virConnectPtr conn, char *name);

void show_domains(virConnectPtr conn);

void show_vm_info(char *name, virConnectPtr conn);

void start_domain(virConnectPtr conn, const char *name);

void stop_domain(virConnectPtr conn, const char *name);

void delete_domain(virConnectPtr conn, const char *name);
