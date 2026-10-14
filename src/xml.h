#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

const char* create_domain_xml(xmlBufferPtr buf);

char *xpath_get(xmlDocPtr doc, const char *path);

void get_nvram_path(const char* name);
