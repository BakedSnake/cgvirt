#include <getopt.h>
#include <libvirt/libvirt.h>

extern int TOTAL_VM_COUNT;
extern int TOTAL_NET_COUNT;

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
extern char NVRAM[256];

typedef struct {
    const char *key;
    char **dest;
} key_map_t;

typedef struct Feature {
    char *enabled;
    char *name;
} Feature;

typedef struct Loader {
    char *readonly;
    char *type;
    char *format;
    char *path;
} Loader;

typedef struct Nvram {
    char *nv_template;
    char *template_format;
    char *format;
    char *path;
} Nvram;

typedef struct Firmware {
    Feature *features;
    Loader loader;
    Nvram nvram;
} Firmware;

typedef struct Boot {
    char *device;
} Boot;

typedef struct Machine {
    char *arch;
    char *kind;
    char *value;
} Machine;

typedef struct Os {
    char *os_firmware;
    Firmware firmware;
    Machine type;
    Boot *boot_devices;
} Os;

typedef struct Memory {
    char *unit;
    char *size;
} Memory;

typedef struct Vcpu {
    char *placement;
    char *cores;
} Vcpu;

typedef struct Cpu {
    char *mode;
    char *check;
    char *migratable;
} Cpu;

typedef struct Features {
    char *acpi;
    char *apic;
    char *vmport_state;
    char *smm_state;
} Features;

typedef struct Controller {
    char *type;
    char *index;
    char *model;
} Controller;

typedef struct Disk {
    char *type;
    char *device;
    char *driver_name;
    char *driver_type;
    char *source_file;
    char *target_dev;
    char *target_bus;
} Disk;

typedef struct Sound {
    char *model;
} Sound;

typedef struct Interface {
    char *type;
    char *source_network;
    char *model_type;
    /* Network interface ( there may be missing fields
     * on other kinds of interfaces ) */
} Interface;

typedef struct Graphics {
    char *type;
    char *autoport;
    char *listen_type;
} Graphics;

typedef struct Video {
    char *model_type;
    char *model_heads;
    char *model_primary;
    char *driver_name;
} Video;

typedef struct Channel {
    char *type;
    char *target_type;
    char *target_name;
    char *alias_name;
} Channel;

int argHandle(virConnectPtr conn, int argc, char* argv[]);

void parse_yaml(const char *filepath);

static struct option options[] = {
  {"help",              no_argument,            0, 'h'},
  {"version",           no_argument,            0, 'v'},
  {"create",            required_argument,      0, 'c'},
  {"list",              required_argument,      0, 'l'},
  {"start",             required_argument,      0, 's'},
  {"stop",              required_argument,      0, 'q'},
  {"delete",            required_argument,      0, 'D'},
  {"info",              required_argument,      0, 'i'},
  {"parse",             required_argument,      0, 'p'},
  {0,                   0,                      0,  0 }
};
