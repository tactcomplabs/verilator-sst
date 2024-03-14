#
# src/verilog/verilog.mk
#

VERILATOR_ROOT = $(abspath $(dir $(shell which verilator))../share/verilator)
include $(VERILATOR_ROOT)/include/verilated.mk
ifeq (, $(shell which sst-config))
 $(error "No sst-config in $(PATH), add `sst-config` to your PATH")
endif

VERILOG_SRC_DIR = $(abspath $(CURDIR)/verilog)
VERILOG_SRC_SOURCES := $(wildcard $(VERILOG_SRC_DIR)/*.v)
VERILOG_SOURCES := $(notdir $(VERILOG_SRC_SOURCES))

$(info $$VERILOG_SRC_DIR $(VERILOG_SRC_DIR))
$(info $$VERILOG_SRC_SOURCES $(VERILOG_SRC_SOURCES))
$(info $$VERILOG_SOURCES $(VERILOG_SOURCES))

VERILOG_BUILD_OBJS := $(patsubst %.v, %.o, $(addprefix $(BUILD_DIR)/, $(VERILOG_SOURCES)))

$(info $$VERILOG_BUILD_OBJS $(VERILOG_BUILD_OBJS))

TOP_MODULE=Top
