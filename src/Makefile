export SRC_DIR = $(abspath $(CURDIR))
export BUILD_DIR = $(abspath ../build)
export INSTALL_DIR = $(abspath ../install)

include $(abspath $(CURDIR)/verilatorsst/verilatorsst.mk)
include $(abspath /Users/mjeje/verilator-sst/src/BasicVerilogUartMem/BasicVerilogUartMem.mk)
include $(abspath /Users/mjeje/verilator-sst/src/BasicVerilogCounter/BasicVerilogCounter.mk)

ifdef DEBUG
DEBUG_FLAGS = -O0 -g -DVL_DEBUG=1
endif

SST_CXX = $(shell sst-config --CXX)
SST_CXXFLAGS = $(shell sst-config --ELEMENT_CXXFLAGS)
SST_LDFLAGS = $(shell sst-config --ELEMENT_LDFLAGS)
export BUILD_COMMAND = $(SST_CXX) $(DEBUG_FLAGS) -I$(VERILATORSST_SRC_DIR) -I$(BUILD_DIR) $(SST_CXXFLAGS) $(SST_LDFLAGS) $(VERILATOR_CPPFLAGS)
export VERILOG_BUILD_COMMAND = verilator --cc --build --exe --vpi --public-flat-rw -CFLAGS "-fPIC $(DEBUG_FLAGS)" -LDFLAGS "$(SST_LDFLAGS)" $(VERILOG_MACROS) -Wall -Mdir $(BUILD_DIR) --prefix V$(basename $(notdir $@)) --top-module $(basename $(notdir $@)) -I$(dir $<) $<

.DEFAULT_GOAL := register
.PHONY: register verify clean

$(VERILATORSST_BUILD_LIB): $(VERILOG_BUILD_OBJS) $(VERILATORSST_BUILD_OBJS)  $(SSTELEMENT_BUILD_OBJS)
	mkdir -p $(dir $@)
	$(BUILD_COMMAND) -o $@ $(BUILD_DIR)/*.o

$(VERILATORSST_INSTALL_LIB): $(VERILATORSST_BUILD_LIB)
	install -d $(dir $@)
	install -m 644 $^ $(dir $@)

register: $(VERILATORSST_INSTALL_LIB)
	sst-register -u $(VERILATORSST_NAME)
	sst-register $(VERILATORSST_NAME) $(VERILATORSST_NAME)_LIBDIR=$(INSTALL_DIR)

verify: $(VERILATORSST_INSTALL_LIB)
	sst-info $(VERILATORSST_NAME)

clean:
	rm -Rf $(BUILD_DIR)
