#include <libvirt/libvirt.h>

virDomainPtr *get_all_domains(virConnectPtr conn);

virDomainPtr get_domain(virConnectPtr conn, char *name);

void list_vms(virConnectPtr conn);

void show_vm_info(char *name, virConnectPtr conn);
