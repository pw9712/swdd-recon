# Host build of the WiperCtrl example (stubs instead of a real RTE / BSW).
#   make        build the simulation
#   make test   build and run it (exit code = failed checks)

CC      ?= cc
CFLAGS  ?= -std=c99 -Wall -Wextra -Wpedantic -Werror -O0 -g
INC     := -Isrc -Irte -Iplatform -Itest
SRC     := $(wildcard src/*.c)
TEST    := test/Rte_Stub.c test/sim_main.c
BUILD   := build

.PHONY: all test clean

all: $(BUILD)/wiperctrl_sim

$(BUILD)/wiperctrl_sim: $(SRC) $(TEST) $(wildcard src/*.h rte/*.h platform/*.h test/*.h) | $(BUILD)
	$(CC) $(CFLAGS) $(INC) $(SRC) $(TEST) -o $@

$(BUILD):
	mkdir -p $@

test: $(BUILD)/wiperctrl_sim
	./$(BUILD)/wiperctrl_sim

clean:
	rm -rf $(BUILD)
