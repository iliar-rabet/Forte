CONTIKI_PROJECT = send-mpu
all: $(CONTIKI_PROJECT)

CONTIKI = ../../..

# PROJECT_SOURCEFILES += sf-simple.c

# MAKE_WITH_SECURITY ?= 0 # force Security from command line
 
# CFLAGS += -ffunction-sections
# LDFLAGS += -Wl,--gc-sections,--undefined=_reset_vector__,--undefined=InterruptVectors,--undefined=_copy_data_init__,--undefined=_clear_bss_init__,--undefined=_end_of_init__

MODULES += os/services/orchestra
MAKE_MAC = MAKE_MAC_TSCH

# include $(CONTIKI)/Makefile.dir-variables
# MODULES += $(CONTIKI_NG_MAC_DIR)/tsch/sixtop

# ifeq ($(MAKE_WITH_SECURITY),1)
# CFLAGS += -DWITH_SECURITY=1
# endif

# MODULES_REL += $(TARGET)

include $(CONTIKI)/Makefile.include
