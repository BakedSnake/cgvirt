#include <stdio.h>
#include <string.h>
#include <libxml/xmlwriter.h>

#include "cgvt.h"

int TOTAL_VM_COUNT = 0;

char *NAME = NULL;
char *ARCH = NULL;
char *BOOT = NULL;
char *CPU  = NULL;
char *CD   = NULL;
char *MEMORY  = NULL;
char *DISK = NULL;
char *NET = NULL;
char *MODEL = NULL;
char *GRAPHICS = NULL;

int argHandle(virConnectPtr conn, int argc, char* argv[])
{
    int opt;
    const char * arg_str = "hvli:a:b:c:d:g:r:m:n:w:o:s:q:O:x";
    while ((opt = getopt_long(argc, argv, arg_str, options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                // TODO
            case 'v':
                // TODO
            case 'a':
                ARCH = optarg;
                break;
            case 'b':
                BOOT = optarg;
                break;
            case 'c':
                CPU = optarg;
                break;
            case 'd':
                DISK = optarg;
                break;
            case 'w':
                NET = optarg;
                break;
            case 'm':
                MODEL = optarg;
                break;
            case 'g':
                GRAPHICS = optarg;
                break;
            case 'n':
                NAME = optarg;
                break;
            case 'r':
                MEMORY = optarg;
                break;
            case 'O':
                CD = optarg;
                break;
            case 'l':
                fprintf(stdout, "List:\n");
                show_domains(conn);
                fprintf(stdout, "\n");
                return 0;
            case 'i':
                fprintf(stdout, "%s:\n", optarg);
                show_vm_info(optarg, conn);
                return 0;
            case 's':
                start_domain(conn, optarg);
                return 0;
            case 'q':
                stop_domain(conn, optarg);
                return 0;
            case 'x':
                create_domain(conn);
                break;
            case '?':
                default:
                break;
        }
    }

    return 0;
}

int main(int argc, char **argv) {
    virConnectPtr conn;

    if (argc < 2) {
        fprintf(stderr, "Please provide argument.\n");
        return 1;
    }

    conn = virConnectOpen("qemu:///system");
    if (conn == NULL) {
        fprintf(stderr, "Failed to connect to 'qemu:///system'.\n");
        return 1;
    }

    int e = argHandle(conn, argc, argv);
    if (e != 0) return 0;

    virConnectClose(conn);
    return 0;
}

void create_domain(virConnectPtr conn)
{
    xmlBufferPtr buf = xmlBufferCreate();
    if (!buf) return;
    xmlTextWriterPtr w = xmlNewTextWriterMemory(buf, 0);
    if (!w) {
        fprintf(stderr, "Failed to create writer\n");
        return;
    }

    xmlTextWriterStartDocument(w, NULL, "UTF-8", NULL);
        xmlTextWriterStartElement(w, BAD_CAST "domain");

            xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "kvm");
            xmlTextWriterWriteElement(w, BAD_CAST "name", BAD_CAST NAME);
            xmlTextWriterStartElement(w, BAD_CAST "memory");
                xmlTextWriterWriteAttribute(w, BAD_CAST "unit", BAD_CAST "MiB");
                xmlTextWriterWriteString(w, BAD_CAST MEMORY);
            xmlTextWriterEndElement(w);

            xmlTextWriterStartElement(w, BAD_CAST "vcpu");
                xmlTextWriterWriteAttribute(w, BAD_CAST "placement", BAD_CAST "static");
                xmlTextWriterWriteString(w, BAD_CAST CPU);
            xmlTextWriterEndElement(w);

//            <firmware>
//              <feature enabled="yes" name="enrolled-keys"/>
//              <feature enabled="yes" name="secure-boot"/>
//            </firmware>
//            <loader readonly="yes" secure="yes" type="pflash" format="raw">/usr/share/edk2/OvmfX64/OVMF_CODE.secboot.fd</loader>
//            <nvram template="/usr/share/edk2/OvmfX64/OVMF_VARS.secboot.fd" templateFormat="raw" format="raw">/var/lib/libvirt/qemu/nvram/susieline_VARS.fd</nvram>

            xmlTextWriterStartElement(w, BAD_CAST "os");
                xmlTextWriterWriteAttribute(w, BAD_CAST "firmware", BAD_CAST "efi");
                xmlTextWriterStartElement(w, BAD_CAST "type");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "arch", BAD_CAST ARCH);
                    xmlTextWriterWriteAttribute(w, BAD_CAST "machine", BAD_CAST "q35");
                    xmlTextWriterWriteString(w, BAD_CAST "hvm");
                xmlTextWriterEndElement(w);
                xmlTextWriterStartElement(w, BAD_CAST "boot");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "dev", BAD_CAST "hd");
                xmlTextWriterEndElement(w);
            xmlTextWriterEndElement(w);

            xmlTextWriterStartElement(w, BAD_CAST "features");
                xmlTextWriterStartElement(w, BAD_CAST "acpi");
                xmlTextWriterEndElement(w);
                xmlTextWriterStartElement(w, BAD_CAST "apic");
                xmlTextWriterEndElement(w);
                xmlTextWriterStartElement(w, BAD_CAST "vmport");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "state", BAD_CAST "off");
                xmlTextWriterEndElement(w);
                xmlTextWriterStartElement(w, BAD_CAST "smm");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "state", BAD_CAST "on");
                xmlTextWriterEndElement(w);
            xmlTextWriterEndElement(w);

            xmlTextWriterStartElement(w, BAD_CAST "cpu");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "mode", BAD_CAST "host-passthrough");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "check", BAD_CAST "none");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "migratable", BAD_CAST "on");
            xmlTextWriterEndElement(w);

            xmlTextWriterStartElement(w, BAD_CAST "devices");

                xmlTextWriterWriteElement(w, BAD_CAST "emulator",
                    BAD_CAST "/usr/bin/qemu-system-x86_64");

                xmlTextWriterStartElement(w, BAD_CAST "video");
                    xmlTextWriterStartElement(w, BAD_CAST "model");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "virtio");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "heads", BAD_CAST "1");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "primary", BAD_CAST "yes");
                    xmlTextWriterEndElement(w);
                    xmlTextWriterStartElement(w, BAD_CAST "driver");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "name", BAD_CAST "qemu");
                    xmlTextWriterEndElement(w);
                xmlTextWriterEndElement(w);

                xmlTextWriterStartElement(w, BAD_CAST "controller");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "scsi");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "index", BAD_CAST "0");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "model", BAD_CAST "virtio-scsi");
                xmlTextWriterEndElement(w);

                xmlTextWriterStartElement(w, BAD_CAST "serial");
                xmlTextWriterEndElement(w);

                xmlTextWriterStartElement(w, BAD_CAST "sound");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "model", BAD_CAST "ich9");
                xmlTextWriterEndElement(w);

                xmlTextWriterStartElement(w, BAD_CAST "rng");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "model", BAD_CAST "virtio");
                    xmlTextWriterStartElement(w, BAD_CAST "backend");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "model", BAD_CAST "random");
                        xmlTextWriterWriteString(w, BAD_CAST "/dev/urandom");
                    xmlTextWriterEndElement(w);
                xmlTextWriterEndElement(w);

                xmlTextWriterStartElement(w, BAD_CAST "disk");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "file");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "device", BAD_CAST "disk");
                xmlTextWriterStartElement(w, BAD_CAST "driver");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "name", BAD_CAST "qemu");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "qcow2");
                xmlTextWriterEndElement(w);

                xmlTextWriterStartElement(w, BAD_CAST "source");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "file", BAD_CAST DISK);
                    xmlTextWriterEndElement(w);
                    xmlTextWriterStartElement(w, BAD_CAST "target");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "dev", BAD_CAST "vda");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "bus", BAD_CAST "virtio");
                    xmlTextWriterEndElement(w);
                xmlTextWriterEndElement(w);

                xmlTextWriterStartElement(w, BAD_CAST "disk");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "file");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "device", BAD_CAST "cdrom");
                xmlTextWriterStartElement(w, BAD_CAST "driver");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "name", BAD_CAST "qemu");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "raw");
                xmlTextWriterEndElement(w);

                xmlTextWriterStartElement(w, BAD_CAST "source");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "file", BAD_CAST CD);
                    xmlTextWriterEndElement(w);
                    xmlTextWriterStartElement(w, BAD_CAST "target");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "dev", BAD_CAST "sda");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "bus", BAD_CAST "scsi");
                    xmlTextWriterEndElement(w);
                xmlTextWriterEndElement(w);

                xmlTextWriterStartElement(w, BAD_CAST "interface");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "network");
                    xmlTextWriterStartElement(w, BAD_CAST "source");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "network", BAD_CAST NET);
                    xmlTextWriterEndElement(w);
                    xmlTextWriterStartElement(w, BAD_CAST "model");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST MODEL);
                    xmlTextWriterEndElement(w);
                xmlTextWriterEndElement(w);

                xmlTextWriterStartElement(w, BAD_CAST "channel");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "spicevmc");
                    xmlTextWriterStartElement(w, BAD_CAST "target");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "virtio");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "name", BAD_CAST "com.redhat.spice.0");
                    xmlTextWriterEndElement(w);
                    xmlTextWriterStartElement(w, BAD_CAST "alias");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "name", BAD_CAST "channel1");
                    xmlTextWriterEndElement(w);
                xmlTextWriterEndElement(w);

                xmlTextWriterStartElement(w, BAD_CAST "graphics");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST GRAPHICS);
                    xmlTextWriterWriteAttribute(w, BAD_CAST "autoport", BAD_CAST "yes");
                    xmlTextWriterStartElement(w, BAD_CAST "listen");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "address");
                    xmlTextWriterEndElement(w);
                xmlTextWriterEndElement(w);

                //xmlTextWriterStartElement(w, BAD_CAST "controller");
                //    xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "pci");
                //    xmlTextWriterWriteAttribute(w, BAD_CAST "index", BAD_CAST "0");
                //    xmlTextWriterWriteAttribute(w, BAD_CAST "model", BAD_CAST "pci-root");
                //xmlTextWriterEndElement(w);

            xmlTextWriterEndElement(w);

        xmlTextWriterEndElement(w); /* </domain> */
    xmlTextWriterEndDocument(w);
    xmlTextWriterFlush(w);

    printf("%s\n", (char *)xmlBufferContent(buf));
    const char* xml = (char *)xmlBufferContent(buf);
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
                virDomainUndefine(doms[i]);
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

void show_domains(virConnectPtr conn)
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
