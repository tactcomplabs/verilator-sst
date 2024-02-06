#
# src/Makefile
#

VERILATOR_ROOT = $(abspath $(dir $(shell which verilator))../share/verilator)
include $(VERILATOR_ROOT)/include/verilated.mk
ifeq (, $(shell which sst-config))
 $(error "No sst-config in $(PATH), add `sst-config` to your PATH")
endif

NPROC = $(shell sysctl -n hw.physicalcpu)

SRC_DIR = $(abspath $(CURDIR))
BUILD_DIR = $(abspath ../build)
INSTALL_DIR = $(abspath ../install)

COMOPNENT_NAME = basicComponent

COMPONENT_LIB=lib$(COMOPNENT_NAME).so
BUILD_LIB=$(BUILD_DIR)/$(COMPONENT_LIB)
INSTALL_LIB=$(INSTALL_DIR)/$(COMPONENT_LIB)

BASICCOMPONENT_SOURCES := $(wildcard *.cc)
SRC_SOURCES := $(SRC_DIR)/$(BASICCOMPONENT_SOURCES)

BASICCOMPONENT_HEADERS := $(patsubst %.cc,%.h, $(BASICCOMPONENT_SOURCES))
SRC_HEADERS := $(SRC_DIR)/$(BASICCOMPONENT_HEADERS)

INSTALL_HEADERS := $(INSTALL_DIR)/$(BASICCOMPONENT_HEADERS) 
BUILD_OBJS := $(patsubst %.cc,%.o, $(BUILD_DIR)/$(BASICCOMPONENT_SOURCES))

OPTIMIZE_FLAGS=-O3
SST_CXX=$(shell sst-config --CXX)
SST_CXXFLAGS = $(shell sst-config --ELEMENT_CXXFLAGS)
SST_LDFLAGS = $(shell sst-config --ELEMENT_LDFLAGS)
SST_CPPFLAGS = -I$(BUILD_DIR)

.PHONY: clean register verify

all: register

$(BUILD_OBJS): $(SRC_SOURCES)
	mkdir -p $(dir $@)
	verilator \
	--cc \
	--build \
	--exe \
	$(OPTIMIZE_FLAGS) \
	-CFLAGS "$(SST_CXXFLAGS)" \
	-LDFLAGS "$(SST_LDFLAGS)" \
	-j $(NPROC) \
	$(VERILOG_MACROS) \
	-Wall \
	--prefix Top \
	--top-module Top \
	-Mdir $(BUILD_DIR) \
	$< top.v

$(BUILD_LIB): $(BUILD_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(OPTIMIZE_FLAGS) $(CXXFLAGS) $(SST_CXXFLAGS) $(CPPFLAGS) $(SST_CPPFLAGS) $(LDFLAGS) $(SST_LDFLAGS) -o $@ $(BUILD_DIR)/*.o

$(INSTALL_HEADERS): $(SRC_HEADERS)
	mkdir -p $(dir $@)
	cp $^ $@

$(INSTALL_LIB): $(BUILD_LIB)
	echo $(BUILD_LIB)
	mkdir -p $(dir $@)
	cp $^ $@

register: $(INSTALL_HEADERS) $(INSTALL_LIB)
	sst-register -u $(COMOPNENT_NAME)
	sst-register $(COMOPNENT_NAME) $(COMOPNENT_NAME)_LIBDIR=$(INSTALL_DIR)

verify:
	sst-info $(COMPONENT_NAME)

clean:
	rm -Rf build
