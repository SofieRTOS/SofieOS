# Copyright 2019 SiFive, Inc #
# SPDX-License-Identifier: Apache-2.0 #

PROGRAM ?= libkernel.a
COBJS := $(patsubst src/%.c, objs/%.o, $(wildcard src/*.c))
SOBJS := $(patsubst src/%.S, objs/%.o, $(wildcard src/*.S))

all: $(PROGRAM)

$(PROGRAM): $(COBJS) $(SOBJS)
	$(AR) rcs $@ $^

$(COBJS): objs/%.o: src/%.c objdir
	$(CC) $(CFLAGS) -c $< -o $@
	
$(SOBJS): objs/%.o: src/%.S objdir
	$(CC) $(CFLAGS) -c $< -o $@

¡£PHONY: clean
clean:
	-rm -rf $(PROGRAM) objs 

objdir:
	mkdir -p objs