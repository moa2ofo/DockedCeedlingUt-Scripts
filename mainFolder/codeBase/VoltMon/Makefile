# ============================================================
#   Makefile per VoltMonitoring Module
# ============================================================

# Compilatore
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -Ipltf -Icfg

# Cartelle sorgenti
PLTF_DIR := pltf
CFG_DIR  := cfg

# File sorgenti
SRCS := \
    $(PLTF_DIR)/VoltMonitoring.c \
    $(CFG_DIR)/VoltMonitoring_cfg.c

# Output finale
TARGET := VoltMonitoring.out

# Oggetti generati
OBJS := $(SRCS:.c=.o)

# ============================================================
#   Regole principali
# ============================================================

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Regola generica per compilare i .c
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Pulizia
clean:
	rm -f $(PLTF_DIR)/*.o
	rm -f $(CFG_DIR)/*.o
	rm -f $(TARGET)

# Pulizia totale
distclean: clean
	rm -f *~
	rm -f $(PLTF_DIR)/*~
	rm -f $(CFG_DIR)/*~

# Stampa i file compilati (debug)
print:
	@echo "Sorgenti: $(SRCS)"
	@echo "Oggetti : $(OBJS)"

.PHONY: all clean distclean print
