# include <libvirt/libvirt.h>

void list_vms(virConnectPtr conn);

void get_vm_info(char *name, virConnectPtr conn);
