DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

HAVE_BANKING=1
UIP_CONF_IPV6=1
UIP_CONF_RPL=1
OFFSET_FIRMWARE=1

PROJECT_SOURCEFILES += ping6.c

CONTIKI_PROJECT = client server  atividade6b

all: $(CONTIKI_PROJECT) 

CONTIKI = ../../..

include $(CONTIKI)/Makefile.include
