#include <libxml/xmlwriter.h>

void get_mem_xml(xmlTextWriterPtr w);

void get_vcpu_xml(xmlTextWriterPtr w);

void get_firmware_xml(xmlTextWriterPtr w);

void get_features_xml(xmlTextWriterPtr w);

void get_cpu_conf_xml(xmlTextWriterPtr w);

const char* get_domain_xml(xmlBufferPtr buf);
