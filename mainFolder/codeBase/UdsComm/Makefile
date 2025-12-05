# ============================================================
#   Makefile per Diagnostic Module
# ============================================================

# Compilatore
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -Ipltf -Icfg

# Cartelle sorgenti
PLTF_DIR := pltf
CFG_DIR  := cfg

# File sorgenti
SRCS := \
    $(PLTF_DIR)/diagnostic.c \
    $(CFG_DIR)/diagnostic_cfg.c

# Output finale
TARGET := diagnostic.out

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
