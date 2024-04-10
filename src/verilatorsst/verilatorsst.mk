ifeq (, $(shell which verilator))
$(error "No verilator in $(PATH), add `verilator` to your PATH")
endif
VERILATOR_ROOT = $(abspath $(dir $(shell which verilator))../share/verilator)
include $(VERILATOR_ROOT)/include/verilated.mk

VM_SC = 0
VM_TRACE = 0
VERILATOR_CPPFLAGS = $(VK_CPPFLAGS_ALWAYS)
VERILATOR_LDFLAGS = $(LDFLAGS)

VERILATORSST_SRC_DIR = $(abspath $(CURDIR)/verilatorsst)
VERILATORSST_BUILD_OBJS := $(patsubst %.cc, %.o, $(addprefix $(BUILD_DIR)/,  $(notdir $(wildcard $(abspath $(CURDIR)/verilatorsst)/*.cc))))
VERILATORSST_NAME = verilatorsst
VERILATORSST_LIB = lib$(VERILATORSST_NAME).so
VERILATORSST_BUILD_LIB = $(BUILD_DIR)/$(VERILATORSST_LIB)
VERILATORSST_INSTALL_LIB = $(addprefix $(INSTALL_DIR)/, $(VERILATORSST_LIB))

$(BUILD_DIR)/%.o: $(VERILATORSST_SRC_DIR)/%.cc
	mkdir -p $(dir $@)
	$(BUILD_COMMAND) -c $< -o $@
	