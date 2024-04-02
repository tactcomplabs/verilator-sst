#
# src/Makefile
#

NPROC = $(shell sysctl -n hw.physicalcpu)

export SRC_DIR = $(abspath $(CURDIR))
export BUILD_DIR = $(abspath ../build)
export INSTALL_DIR = $(abspath ../install)

DEBUG ?=
DEBUG_FLAGS =
ifdef DEBUG
DEBUG_FLAGS = -O0 -g -DVL_DEBUG=1
endif

include $(abspath $(CURDIR)/verilog/verilog.mk)
include $(abspath $(CURDIR)/sstelement/sstelement.mk)

.DEFAULT_GOAL := register
.PHONY: register verify clean

$(VERILOG_BUILD_HEADERS): $(VERILOG_SRC_SOURCES)
	mkdir -p $(dir $@)
	verilator \
	--cc \
	--exe \
	--vpi \
	--public-flat-rw \
	-CFLAGS "-fPIC $(DEBUG_FLAGS)" \
	-LDFLAGS "$(SST_LDFLAGS)" \
	-j $(NPROC) \
	$(VERILOG_MACROS) \
	-Wall \
	--prefix VTop \
	--top-module $(TOP_MODULE) \
	-Mdir $(BUILD_DIR) \
	$^
	make -C $(dir $@) -j $(NPROC) -f VTop.mk

$(BUILD_DIR)/%.o: $(SRC_DIR)/sstelement/%.cc $(VERILOG_BUILD_HEADERS)
	mkdir -p $(dir $@)
	$(SST_CXX) $(DEBUG_FLAGS) $(SST_CXXFLAGS) $(VERILATOR_CPPFLAGS) $(SST_LDFLAGS) $(VERILATOR_LDFLAGS) -c $< -o $@

$(SSTELEMENT_BUILD_LIB): $(SSTELEMENT_BUILD_OBJS) 
	mkdir -p $(dir $@)
	$(SST_CXX) $(DEBUG_FLAGS) $(SST_CXXFLAGS) $(VERILATOR_CPPFLAGS) $(SST_LDFLAGS) $(VERILATOR_LDFLAGS) -o $@ $(BUILD_DIR)/*.o

$(SSTELEMENT_INSTALL_HEADERS): $(SSTELEMENT_SRC_HEADERS)
	install -d $(dir $@)
	install -m 644 $^ $(dir $@)

$(SSTELEMENT_INSTALL_LIB): $(SSTELEMENT_BUILD_LIB)
	install -d $(dir $@)
	install -m 644 $^ $(dir $@)

register: $(SSTELEMENT_INSTALL_LIB)
	sst-register -u $(SSTELEMENT_NAME)
	sst-register $(SSTELEMENT_NAME) $(SSTELEMENT_NAME)_LIBDIR=$(INSTALL_DIR)

verify: $(SSTELEMENT_INSTALL_LIB)
	sst-info $(SSTELEMENT_NAME)

clean:
	rm -Rf $(BUILD_DIR)
