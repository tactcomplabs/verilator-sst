#
# src/verilog/verilog.mk
#

VERILATOR_ROOT = $(abspath $(dir $(shell which verilator))../share/verilator)
include $(VERILATOR_ROOT)/include/verilated.mk
ifeq (, $(shell which sst-config))
 $(error "No sst-config in $(PATH), add `sst-config` to your PATH")
endif

VM_SC = 0
VM_TRACE = 0
VERILATOR_CPPFLAGS = $(VK_CPPFLAGS_ALWAYS)
VERILATOR_LDFLAGS = $(LDFLAGS)
$(info $$VERILATOR_CPPFLAGS $(VERILATOR_CPPFLAGS))
$(info $$VERILATOR_LDFLAGS $(VERILATOR_LDFLAGS))

VERILOG_SRC_DIR = $(abspath $(CURDIR)/verilog)
VERILOG_SRC_SOURCES := $(wildcard $(VERILOG_SRC_DIR)/*.v)
VERILOG_SOURCES := $(notdir $(VERILOG_SRC_SOURCES))
VERILOG_BUILD_HEADERS := $(patsubst %.v, %.h, $(addprefix $(BUILD_DIR)/, $(VERILOG_SOURCES)))

$(info $$VERILOG_SRC_DIR $(VERILOG_SRC_DIR))
$(info $$VERILOG_SRC_SOURCES $(VERILOG_SRC_SOURCES))
$(info $$VERILOG_SOURCES $(VERILOG_SOURCES))
$(info $$VERILOG_BUILD_HEADERS $(VERILOG_BUILD_HEADERS))

TOP_MODULE=Top
