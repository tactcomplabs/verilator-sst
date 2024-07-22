#ifndef _Test_HARNESS_H_
#define _Test_HARNESS_H_

#include "TestMacros.h"

// -- Verilator SST Headers
#include "verilatorSSTSubcomponent.h"

namespace SST::VerilatorSST {

class TestHarness : public SST::Component {
  // TestHarness: test suite function. 
  // Called on every SST cycle. 
  // Must be defined by derived test bench class.
  virtual void runTestSuite(SST::Cycle_t currentCycle) = 0;
  std::vector<uint8_t> generateRandomData(unsigned width, unsigned depth);

  uint32_t numCycles; // TestHarness: number of cycles to execute

protected:
  void testDefault();
  SST::Output    *output;                   // TestHarness: SST output
  SST::VerilatorSST::VerilatorSSTBase *dut; // TestHarness: subcomponent dut

public:
  // TestHarness: constuctor
  TestHarness(SST::ComponentId_t id, const SST::Params& params);

  // TestHarness: destructor
  ~TestHarness();

  // TestHarness: setup function
  void setup();

  // TestHarness: finish function
  void finish();

  // TestHarness: init function
  void init( unsigned int phase );

  // TestHarness: clock function
  bool clock(SST::Cycle_t currentCycle );

  // -------------------------------------------------------
  // TestHarness Component Parameter Data
  // -------------------------------------------------------
  // clang-format off
  SST_ELI_DOCUMENT_PARAMS(
    {"verbose",     "Sets the verbosity",       "0"},
    {"clockFreq",   "Clock frequency",          "1GHz"},
    {"numCycles",   "Number of cycles to exec", "1000"},
  )

  // -------------------------------------------------------
  // TestHarness Port Parameter Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_PORTS(
  )

  // -------------------------------------------------------
  // TestHarness SubComponent Parameter Data
  // -------------------------------------------------------
  SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
    {"dut", "Verilator Subcomponent Model",   "SST::VerilatorSST::VerilatorSSTBase"},
  )
};

}
#endif
