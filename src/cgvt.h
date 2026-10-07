#include <getopt.h>
#include <libvirt/libvirt.h>

extern int TOTAL_VM_COUNT;

extern char *NAME;
extern char *ARCH;
extern char *BOOT;
extern char *CPU;
extern char *CD;
extern char *MEMORY;
extern char *DISK;
extern char *NET;
extern char *MODEL;
extern char *GRAPHICS;


virDomainPtr *get_all_domains(virConnectPtr conn);

virDomainPtr get_domain(virConnectPtr conn, char *name);

void show_domains(virConnectPtr conn);

void show_vm_info(char *name, virConnectPtr conn);

void start_domain(virConnectPtr conn, const char *name);

void stop_domain(virConnectPtr conn, const char *name);

void delete_domain(virConnectPtr conn, const char *name);

void create_domain(virConnectPtr conn);

static struct option options[] = {
  {"help",              no_argument,            0, 'h'},
  {"version",           no_argument,            0, 'v'},
  {"create",            no_argument,            0, 'x'},
  {"list",              no_argument,            0, 'l'},
  {"start",             required_argument,      0, 's'},
  {"stop",              required_argument,      0, 'q'},

  {"info",              required_argument,      0, 'i'},
  {"arch",              required_argument,      0, 'a'},
  {"boot",              required_argument,      0, 'b'},
  {"cpu",               required_argument,      0, 'c'},
  {"cd",                required_argument,      0, 'O'},
  {"disk",              required_argument,      0, 'd'},
  {"graphics",          required_argument,      0, 'g'},
  {"memory",            required_argument,      0, 'r'},
  {"model",             required_argument,      0, 'm'},
  {"name",              required_argument,      0, 'n'},
  {"net",               required_argument,      0, 'w'},
  {"os",                required_argument,      0, 'o'},
  {0,                   0,                      0,  0 }
};
