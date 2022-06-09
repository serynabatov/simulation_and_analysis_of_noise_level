CONTIKI_PROJECT = controller
all: $(CONTIKI_PROJECT)
CONTIKI = /home/user/Desktop/contiki-ng-mw-2122

# Include RPL BR module
MODULES += os/services/rpl-border-router

include $(CONTIKI)/Makefile.include

PREFIX ?= fd00::1/64

connect-router-ACM0:	$(CONTIKI)/tools/serial-io/tunslip6
	sudo $(CONTIKI)/tools/serial-io/tunslip6 -L -s ttyACM0 $(PREFIX)

connect-router-ACM2:	$(CONTIKI)/tools/serial-io/tunslip6
	sudo $(CONTIKI)/tools/serial-io/tunslip6 -L -s ttyACM2 $(PREFIX)
