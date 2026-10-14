#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cobia.h"
#include "xml.h"

void get_nvram_path(const char *name)
{
    snprintf(NVRAM, 256, "/var/lib/libvirt/qemu/nvram/%s_VARS.qcow2", name);
}

char *xpath_get(xmlDocPtr doc, const char *path)
{
    char *result = (char *)malloc(256);
    xmlXPathContext *ctx = xmlXPathNewContext(doc);
    xmlXPathObject *obj = xmlXPathEvalExpression((const xmlChar*)path, ctx);

    if (obj && obj->nodesetval && obj->nodesetval->nodeNr > 0) {
        xmlChar *s = xmlNodeGetContent(obj->nodesetval->nodeTab[0]);
        if (s) {
            strcpy(result, (char *)s);
            xmlFree(s);
        }
    }

    xmlXPathFreeObject(obj);
    xmlXPathFreeContext(ctx);
    return result;
}

const char* create_domain_xml(xmlBufferPtr buf)
{
    xmlTextWriterPtr w = xmlNewTextWriterMemory(buf, 0);
    if (!w) {
        fprintf(stderr, "Failed to create writer\n");
        return NULL;
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

            xmlTextWriterStartElement(w, BAD_CAST "os");
                xmlTextWriterWriteAttribute(w, BAD_CAST "firmware", BAD_CAST "efi");
                xmlTextWriterStartElement(w, BAD_CAST "firmware");
                    xmlTextWriterStartElement(w, BAD_CAST "feature");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "enabled", BAD_CAST SECBOOT);
                        xmlTextWriterWriteAttribute(w, BAD_CAST "name", BAD_CAST "enrolled-keys");
                    xmlTextWriterEndElement(w);
                    xmlTextWriterStartElement(w, BAD_CAST "feature");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "enabled", BAD_CAST SECBOOT);
                        xmlTextWriterWriteAttribute(w, BAD_CAST "name", BAD_CAST "secure-boot");
                    xmlTextWriterEndElement(w);
                    xmlTextWriterStartElement(w, BAD_CAST "loader");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "readonly", BAD_CAST "yes");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "pflash");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "format", BAD_CAST "qcow2");
                        if (strcmp(SECBOOT, "yes") == 0)
                            xmlTextWriterWriteString(w, BAD_CAST "/usr/share/edk2/OvmfX64/OVMF_CODE.secboot.qcow2");
                        else
                            xmlTextWriterWriteString(w, BAD_CAST "/usr/share/edk2/OvmfX64/OVMF_CODE.qcow2");
                    xmlTextWriterEndElement(w);
                    xmlTextWriterStartElement(w, BAD_CAST "nvram");
                        if (strcmp(SECBOOT, "yes") == 0)
                            xmlTextWriterWriteAttribute(w, BAD_CAST "template", BAD_CAST "/usr/share/edk2/OvmfX64/OVMF_VARS.secboot.qcow2");
                        else
                            xmlTextWriterWriteAttribute(w, BAD_CAST "template", BAD_CAST "/usr/share/edk2/OvmfX64/OVMF_VARS.qcow2");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "templateFormat", BAD_CAST "qcow2");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "format", BAD_CAST "qcow2");
                        xmlTextWriterWriteString(w, BAD_CAST NVRAM);
                    xmlTextWriterEndElement(w);
                xmlTextWriterEndElement(w);

                xmlTextWriterStartElement(w, BAD_CAST "type");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "arch", BAD_CAST ARCH);
                    xmlTextWriterWriteAttribute(w, BAD_CAST "machine", BAD_CAST MACHINE);
                    xmlTextWriterWriteString(w, BAD_CAST "hvm");
                xmlTextWriterEndElement(w);
                xmlTextWriterStartElement(w, BAD_CAST "boot");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "dev", BAD_CAST BOOTDEV);
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

                xmlTextWriterWriteElement(w, BAD_CAST "emulator", BAD_CAST "/usr/bin/qemu-system-x86_64");

                xmlTextWriterStartElement(w, BAD_CAST "video");
                    xmlTextWriterStartElement(w, BAD_CAST "model");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST VIDEO_MDL);
                        xmlTextWriterWriteAttribute(w, BAD_CAST "heads", BAD_CAST "1");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "primary", BAD_CAST "yes");
                    xmlTextWriterEndElement(w);
                    xmlTextWriterStartElement(w, BAD_CAST "driver");
                        xmlTextWriterWriteAttribute(w, BAD_CAST "name", BAD_CAST VIDEO_DRV);
                    xmlTextWriterEndElement(w);
                xmlTextWriterEndElement(w);

                xmlTextWriterStartElement(w, BAD_CAST "controller");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "type", BAD_CAST "scsi");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "index", BAD_CAST "0");
                    xmlTextWriterWriteAttribute(w, BAD_CAST "model", BAD_CAST "virtio-scsi");
                xmlTextWriterEndElement(w);

                xmlTextWriterWriteElement(w, BAD_CAST "serial", BAD_CAST "");

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

    const char* xml = (char *)xmlBufferContent(buf);
    return xml;
}
