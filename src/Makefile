#
# src/Makefile
#

NPROC = $(shell sysctl -n hw.physicalcpu)

export SRC_DIR = $(abspath $(CURDIR))
export BUILD_DIR = $(abspath ../build)
export INSTALL_DIR = $(abspath ../install)

include $(abspath $(CURDIR)/verilog/verilog.mk)
include $(abspath $(CURDIR)/sstelement/sstelement.mk)

.DEFAULT_GOAL := register
.PHONY: register verify clean

$(SSTELEMENT_BUILD_OBJS): $(VERILOG_SRC_SOURCES) $(SSTELEMENT_SRC_SOURCES) 
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
	--top-module $(TOP_MODULE) \
	-Mdir $(BUILD_DIR) \
	$< $(SSTELEMENT_SRC_SOURCES)

$(SSTELEMENT_BUILD_LIB): $(SSTELEMENT_BUILD_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(OPTIMIZE_FLAGS) $(CXXFLAGS) $(SST_CXXFLAGS) $(CPPFLAGS) $(SST_CPPFLAGS) $(LDFLAGS) $(SST_LDFLAGS) -o $@ $(SSTELEMENT_BUILD_OBJS)

$(SSTELEMENT_INSTALL_HEADERS): $(SSTELEMENT_SRC_HEADERS)
	install -d $(dir $@)
	install -m 644 $^ $(dir $@)

$(SSTELEMENT_INSTALL_LIB): $(SSTELEMENT_BUILD_LIB)
	install -d $(dir $@)
	install -m 644 $^ $(dir $@)

register: $(SSTELEMENT_INSTALL_LIB) $(SSTELEMENT_INSTALL_HEADERS)
	sst-register -u $(SSTELEMENT_NAME)
	sst-register $(SSTELEMENT_NAME) $(SSTELEMENT_NAME)_LIBDIR=$(INSTALL_DIR)

verify: $(SSTELEMENT_INSTALL_LIB)
	sst-info $(SSTELEMENT_NAME)

clean:
	rm -Rf $(BUILD_DIR)
