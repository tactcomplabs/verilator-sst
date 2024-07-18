//
// _verilatorSSTSubcomponent_cpp_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "verilatorSSTSubcomponent.h"
#include "Signal.h"

using namespace SST::VerilatorSST;

// ---------------------------------------------------------------
// VerilatorSSTBase
// ---------------------------------------------------------------
VerilatorSSTBase::VerilatorSSTBase( std::string DerivedName,
                                    ComponentId_t id, const Params& params )
  : SubComponent(id), output(nullptr){
  verbosity = params.find<uint32_t>("verbose");
  std::string outStr = "[" + DerivedName + " @t]: ";
  output = new SST::Output(outStr, verbosity, 0,
                           SST::Output::STDOUT);
}

VerilatorSSTBase::~VerilatorSSTBase(){
  delete output;
}

// ---------------------------------------------------------------
// VerilatorSSTDUT
// ---------------------------------------------------------------
VerilatorSSTDUT::VerilatorSSTDUT(ComponentId_t id,
                                                           const Params& params)
  : VerilatorSSTBase("DUT", id, params), UseVPI(false){

  UseVPI = params.find<bool>("useVPI", false);
  const std::string clockFreq = params.find<std::string>("clockFreq", "1GHz");

  // init verilator interfaces
  ContextP = new VerilatedContext();
  ContextP->threads(1);
  ContextP->debug(VL_DEBUG);
  ContextP->randReset(2);
  ContextP->traceEverOn(true);
  const char *empty {};
  ContextP->commandArgs(0,&empty);
  Top = new VTop(ContextP, "");
#if VL_DEBUG == 1
  ContextP->internalsDump();
#endif

  // attempt to build the reset value tables
  initResetValues(params);
  

  // register the clock
  registerClock(clockFreq,
                new Clock::Handler<VerilatorSSTDUT>(this,
                                                                 &VerilatorSSTDUT::clock));
}

VerilatorSSTDUT::~VerilatorSSTDUT(){
  delete Top;
  delete ContextP;
}

void VerilatorSSTDUT::splitStr(const std::string& s,
                                            char c,
                                            std::vector<std::string>& v){
  std::string::size_type i = 0;
  std::string::size_type j = s.find(c);
  v.clear();

  if( (j==std::string::npos) && (s.length() > 0) ){
    v.push_back(s);
    return ;
  }

  while (j != std::string::npos) {
    v.push_back(s.substr(i, j-i));
    i = ++j;
    j = s.find(c, j);
    if (j == std::string::npos)
      v.push_back(s.substr(i, s.length()));
  }
}

void VerilatorSSTDUT::pollWriteQueue(){
  uint64_t currTick = getCurrentTick();
  for (auto it=WriteQueue.begin(); it!=WriteQueue.end();) {
    auto ele = *it;
    if (ele.AtTick == currTick) {
      if (UseVPI) {
        writePortVPI(ele.PortName, ele.Packet);
      } else {
        unsigned Idx = PortMap.at(ele.PortName);
        DirectWriteFunc Func = std::get<V_WRITEFUNC>(Ports[Idx]);
        (*Func)(Top,ele.Packet);
      }
      it = WriteQueue.erase(it);
    } else {
      it++;
    }
  }
}

void VerilatorSSTDUT::initResetValues(const Params& params){
  std::vector<std::string> optList;
  params.find_array("resetVals", optList);

  for( unsigned i=0; i<optList.size(); i++ ){
    std::vector<std::string> vstr;
    std::string s = optList[i];
    splitStr(s, ':', vstr);

    if( vstr.size() != 2 ){
      output->fatal(CALL_INFO, -1,
                    "Error in reading reset value from parameter list:%s\n",
                    s.c_str() );
    }

    if( !isNamedPort(vstr[0]) ){
      output->fatal(CALL_INFO, -1,
                    "Error in reading reset value: %s is not a named port\n",
                    vstr[0].c_str());
    }

    long unsigned val = std::stoul(vstr[1]);
    ResetVals.push_back(std::make_pair(vstr[0],val));
  }
}

void VerilatorSSTDUT::init(unsigned int phase){
  for (auto ele : ResetVals) {
    std::vector<uint8_t> d;
    // convert uint64 to byte vector
    output->verbose(CALL_INFO, 1, 0, "initializing port %s to %lx\n", ele.first.data(), ele.second);
    for (int i=0; i<8; i++) {
      uint8_t tmp = (ele.second >> (i*8)) & 255;
      d.push_back(tmp);
    }
    writePort(ele.first, d);
  }
}

void VerilatorSSTDUT::setup(){
}

void VerilatorSSTDUT::finish(){
  Top->final();
}

bool VerilatorSSTDUT::clock(SST::Cycle_t cycle){
  // apply queued writes
  pollWriteQueue();
  ContextP->timeInc(1);
  Top->eval();
  return false;
}

bool VerilatorSSTDUT::isNamedPort(std::string PortName){

  for( unsigned i=0; i<Ports.size(); i++ ){
    if( std::get<V_NAME>(Ports[i]) == PortName ){
      return true;
    }
  }

  return false;
}

unsigned VerilatorSSTDUT::getNumPorts(){
  return Ports.size();
}

const std::vector<std::string> VerilatorSSTDUT::getPortsNames(){
  std::vector<std::string> Names;

  for( unsigned i=0; i<Ports.size(); i++ ){
    Names.push_back(std::get<V_NAME>(Ports[i]));
  }

  return Names;
}

bool VerilatorSSTDUT::getPortType(std::string PortName,
                                               SST::VerilatorSST::VPortType& Type){
  unsigned Idx = 0;
  try{
    Idx = PortMap.at(PortName);
  }catch(const std::out_of_range& oor){
    output->fatal(CALL_INFO, -1, "Could not find port with name=%s\n",
                  PortName.c_str());
    return false;
  }

  Type = std::get<V_TYPE>(Ports[Idx]);
  return true;
}

bool VerilatorSSTDUT::getPortWidth(std::string PortName,
                                                unsigned& Width){
  unsigned Idx = 0;
  try{
    Idx = PortMap.at(PortName);
  }catch(const std::out_of_range& oor){
    output->fatal(CALL_INFO, -1, "Could not find port with name=%s\n",
                  PortName.c_str());
    return false;
  }

  Width = std::get<V_WIDTH>(Ports[Idx]);
  return true;
}

bool VerilatorSSTDUT::getPortDepth(std::string PortName,
                                                unsigned& Depth){
  unsigned Idx = 0;
  try{
    Idx = PortMap.at(PortName);
  }catch(const std::out_of_range& oor){
    output->fatal(CALL_INFO, -1, "Could not find port with name=%s\n",
                  PortName.c_str());
    return false;
  }

  Depth = std::get<V_DEPTH>(Ports[Idx]);
  return true;
}

bool VerilatorSSTDUT::getResetVal(std::string PortName,
                                               uint64_t& Val){
  unsigned Idx = 0;
  try{
    Idx = PortMap.at(PortName);
  }catch(const std::out_of_range& oor){
    output->fatal(CALL_INFO, -1, "Could not find port with name=%s\n",
                  PortName.c_str());
    return false;
  }

  for( unsigned i=0; i<ResetVals.size(); i++ ){
    if( ResetVals[i].first == PortName ){
      Val = ResetVals[i].second;
      return true;
    }
  }

  return false;
}

uint64_t VerilatorSSTDUT::getCurrentTick(){
  return ContextP->time();
}

std::vector<uint8_t> VerilatorSSTDUT::readPortVPI(std::string PortName){
  vpiHandle vh1 = vpi_handle_by_name((PLI_BYTE8 *)PortName.data(), NULL);
  assert(vh1 && "vpi should return a handle (port not found)");

  auto vpiTypeVal = vpi_get(vpiType, vh1);
  auto vpiSizeVal = vpi_get(vpiSize, vh1);

  if(vpiTypeVal == vpiReg){
    s_vpi_value val{vpiVectorVal};
    vpi_get_value(vh1, &val);
    
    auto signalFactory = SignalFactory(vpiSizeVal,1);
    auto signalPtr = signalFactory(val);
    const std::vector<uint8_t>& d = signalPtr->getUIntVector(true);
    delete signalPtr;
    return d;
  }

  if(vpiTypeVal == vpiMemory){
    vpiHandle iter = vpi_iterate(vpiMemoryWord,vh1);
    assert(iter);

    SignalFactory * signalFactory;
    Signal * signalPtr = nullptr;
    auto i = 0;
    while(auto rowHandle = vpi_scan(iter)){
      if(i == 0){
        const auto rowSizeBits = vpi_get(vpiSize, rowHandle);
        signalFactory = new SignalFactory(rowSizeBits, vpiSizeVal);
      }

      s_vpi_value row{SIGNAL_VPI_FORMAT};
      vpi_get_value(rowHandle, &row);
      signalPtr = (*signalFactory)(row);

      vpi_free_object(rowHandle);
      i++;
    }

    const std::vector<uint8_t>& d = signalPtr->getUIntVector(true);
    delete signalPtr;
    delete signalFactory;
    return d;
  }

  assert(false && "unsupported vpiType");
  std::vector<uint8_t> d;
  return d;
}

void VerilatorSSTDUT::writePortVPI(std::string PortName,
                                                const std::vector<uint8_t>& Packet){
  vpiHandle vh1 = vpi_handle_by_name(PortName.data(), NULL);
  assert(vh1 && "vpi should return a handle (port not found)");

  auto vpiTypeVal = vpi_get(vpiType, vh1);
  auto vpiSizeVal = vpi_get(vpiSize, vh1);
  

  auto vpiDirVal = vpi_get(vpiDirection,vh1);
  assert(vpiDirVal == vpiInput && "port must be an input, inout not supported");
  unsigned Width;
  unsigned Depth;
  getPortWidth(PortName, Width);
  getPortDepth(PortName, Depth);
  Signal toWrite(Width, Depth, Packet, true);
  if(vpiTypeVal == vpiReg){
    t_vpi_value val = toWrite.getVpiValue(0);
    vpi_put_value(vh1,&val,NULL,vpiNoDelay);
    return;
  }

  if(vpiTypeVal == vpiMemory){
    assert(vpiSizeVal == Depth && "port depth must match signal depth");
    vpiHandle iter = vpi_iterate(vpiMemoryWord,vh1);
    assert(iter);

    int i = 0;
    while(auto rowHandle = vpi_scan(iter)){
      auto rowSizeBits = vpi_get(vpiSize, rowHandle);
      assert(rowSizeBits == Width && "row width must match signal width");

      t_vpi_value val = toWrite.getVpiValue(i);
      vpi_put_value(rowHandle,&val,NULL,0);

      vpi_free_object(rowHandle);
      i++;
    }

    return;
  }

  assert(false && "unsupported vpiType");
}

void VerilatorSSTDUT::writePort(std::string PortName,
                                             const std::vector<uint8_t>& Packet){
  // sanity check
  if( !isNamedPort(PortName) ){
    output->fatal(CALL_INFO, -1, "Could not find port with name=%s\n",
                  PortName.c_str());
  }

  // determine which write to use
  if( UseVPI ){
    writePortVPI(PortName, Packet);
  }else{
    unsigned Idx = PortMap.at(PortName);
    DirectWriteFunc Func = std::get<V_WRITEFUNC>(Ports[Idx]);
    (*Func)(Top,Packet);
  }
}

void VerilatorSSTDUT::writePortAtTick(std::string PortName,
                                                   const std::vector<uint8_t>& Packet,
                                                   uint64_t Tick){
  // sanity check
  if( !isNamedPort(PortName) ){
    output->fatal(CALL_INFO, -1, "Could not find port with name=%s\n",
                  PortName.c_str());
  }

  // Tick is used as a delay/offset, not a definite tick value
  // VPI/Direct is decided when polling the WriteQueue
  WriteQueue.emplace_back(PortName, Tick+getCurrentTick(), Packet);
}

std::vector<uint8_t> VerilatorSSTDUT::readPort(std::string PortName){

  // sanity check
  if( !isNamedPort(PortName) ){
    output->fatal(CALL_INFO, -1, "Could not find port with name=%s\n",
                  PortName.c_str());
  }

  // determine which write to use
  if( UseVPI ){
    const std::vector<uint8_t>& data = readPortVPI(PortName);
    return data;
  }else{
    unsigned Idx = PortMap.at(PortName);
    DirectReadFunc Func = std::get<V_READFUNC>(Ports[Idx]);
    const std::vector<uint8_t>& data = (*Func)(Top);
    return data;
  }
}

void VerilatorSSTDUT::DirectWritewrite_port(VTop *T, const std::vector<uint8_t>& Packet){
// less than 8 bit
SignalHelper S(1);
T->write_port = (Packet[0] & S.getMask<uint8_t>());
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadwrite_port(VTop *T){
std::vector<uint8_t> d;
return d;
}
void VerilatorSSTDUT::DirectWritewrite_byte_port(VTop *T, const std::vector<uint8_t>& Packet){
// less than 8 bit
SignalHelper S(8);
T->write_byte_port = (Packet[0] & S.getMask<uint8_t>());
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadwrite_byte_port(VTop *T){
std::vector<uint8_t> d;
return d;
}
void VerilatorSSTDUT::DirectWritewrite_reg_port(VTop *T, const std::vector<uint8_t>& Packet){
// less than 8 bit
SignalHelper S(1);
T->write_reg_port = (Packet[0] & S.getMask<uint8_t>());
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadwrite_reg_port(VTop *T){
std::vector<uint8_t> d;
return d;
}
void VerilatorSSTDUT::DirectWritewrite_wire_port(VTop *T, const std::vector<uint8_t>& Packet){
// less than 8 bit
SignalHelper S(1);
T->write_wire_port = (Packet[0] & S.getMask<uint8_t>());
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadwrite_wire_port(VTop *T){
std::vector<uint8_t> d;
return d;
}
void VerilatorSSTDUT::DirectWritewrite_half_port(VTop *T, const std::vector<uint8_t>& Packet){
// less than 32 bits
SignalHelper S(16);
uint32_t tmp = 0;
tmp = (tmp<<8) + (((uint32_t)Packet[1]) & 255);
tmp = (tmp<<8) + (((uint32_t)Packet[0]) & 255);
T->write_half_port = (tmp & S.getMask<uint32_t>());
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadwrite_half_port(VTop *T){
std::vector<uint8_t> d;
return d;
}
void VerilatorSSTDUT::DirectWritewrite_word_port(VTop *T, const std::vector<uint8_t>& Packet){
// less than 32 bits
SignalHelper S(32);
uint32_t tmp = 0;
tmp = (tmp<<8) + (((uint32_t)Packet[3]) & 255);
tmp = (tmp<<8) + (((uint32_t)Packet[2]) & 255);
tmp = (tmp<<8) + (((uint32_t)Packet[1]) & 255);
tmp = (tmp<<8) + (((uint32_t)Packet[0]) & 255);
T->write_word_port = (tmp & S.getMask<uint32_t>());
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadwrite_word_port(VTop *T){
std::vector<uint8_t> d;
return d;
}
void VerilatorSSTDUT::DirectWritewrite_quad_port(VTop *T, const std::vector<uint8_t>& Packet){
// greater than 64 bits
for (int i=0; i<4; i++) {
uint32_t tmp = 0;
tmp = (tmp<<8) + (((uint32_t)Packet[i*4+3]) & 255);
tmp = (tmp<<8) + (((uint32_t)Packet[i*4+2]) & 255);
tmp = (tmp<<8) + (((uint32_t)Packet[i*4+1]) & 255);
tmp = (tmp<<8) + (((uint32_t)Packet[i*4+0]) & 255);
T->write_quad_port[i] = tmp;
}
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadwrite_quad_port(VTop *T){
std::vector<uint8_t> d;
return d;
}
void VerilatorSSTDUT::DirectWritewrite_double_port(VTop *T, const std::vector<uint8_t>& Packet){
// less than 64 bits
SignalHelper S(64);
uint64_t tmp = 0;
tmp = (tmp<<8) + (((uint64_t)Packet[7]) & 255);
tmp = (tmp<<8) + (((uint64_t)Packet[6]) & 255);
tmp = (tmp<<8) + (((uint64_t)Packet[5]) & 255);
tmp = (tmp<<8) + (((uint64_t)Packet[4]) & 255);
tmp = (tmp<<8) + (((uint64_t)Packet[3]) & 255);
tmp = (tmp<<8) + (((uint64_t)Packet[2]) & 255);
tmp = (tmp<<8) + (((uint64_t)Packet[1]) & 255);
tmp = (tmp<<8) + (((uint64_t)Packet[0]) & 255);
T->write_double_port = (tmp & S.getMask<uint64_t>());
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadwrite_double_port(VTop *T){
std::vector<uint8_t> d;
return d;
}
void VerilatorSSTDUT::DirectWritewrite_array_port(VTop *T, const std::vector<uint8_t>& Packet){
// less than 8 bit
SignalHelper S(1);
for (int i=0; i<128; i++) {
T->write_array_port[i] = (Packet[i] & S.getMask<uint8_t>());
}
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadwrite_array_port(VTop *T){
std::vector<uint8_t> d;
return d;
}
void VerilatorSSTDUT::DirectWritewrite_quad_array_port(VTop *T, const std::vector<uint8_t>& Packet){
// greater than 64 bits
for (int j=0; j<128; j++) {
for (int i=0; i<4; i++) {
uint32_t tmp = 0;
tmp = (tmp<<8) + (((uint32_t)Packet[j*(4*4+0)+i*4+3]) & 255);
tmp = (tmp<<8) + (((uint32_t)Packet[j*(4*4+0)+i*4+2]) & 255);
tmp = (tmp<<8) + (((uint32_t)Packet[j*(4*4+0)+i*4+1]) & 255);
tmp = (tmp<<8) + (((uint32_t)Packet[j*(4*4+0)+i*4+0]) & 255);
T->write_quad_array_port[j][i] = tmp;
}
}
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadwrite_quad_array_port(VTop *T){
std::vector<uint8_t> d;
return d;
}
void VerilatorSSTDUT::DirectWriteread_write_port(VTop *T, const std::vector<uint8_t>& Packet){
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadread_write_port(VTop *T){
std::vector<uint8_t> d;
// less than 8 bit
d.push_back(T->read_write_port);
return d;
}
void VerilatorSSTDUT::DirectWriteread_write_byte_port(VTop *T, const std::vector<uint8_t>& Packet){
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadread_write_byte_port(VTop *T){
std::vector<uint8_t> d;
// less than 8 bit
d.push_back(T->read_write_byte_port);
return d;
}
void VerilatorSSTDUT::DirectWriteread_write_reg_port(VTop *T, const std::vector<uint8_t>& Packet){
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadread_write_reg_port(VTop *T){
std::vector<uint8_t> d;
// less than 8 bit
d.push_back(T->read_write_reg_port);
return d;
}
void VerilatorSSTDUT::DirectWriteread_write_wire_port(VTop *T, const std::vector<uint8_t>& Packet){
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadread_write_wire_port(VTop *T){
std::vector<uint8_t> d;
// less than 8 bit
d.push_back(T->read_write_wire_port);
return d;
}
void VerilatorSSTDUT::DirectWriteread_write_half_port(VTop *T, const std::vector<uint8_t>& Packet){
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadread_write_half_port(VTop *T){
std::vector<uint8_t> d;
// less than 32 bit
uint8_t tmp = 0;
tmp = (T->read_write_half_port >> 0) & 255;
d.push_back(tmp);
tmp = (T->read_write_half_port >> 8) & 255;
d.push_back(tmp);
return d;
}
void VerilatorSSTDUT::DirectWriteread_write_word_port(VTop *T, const std::vector<uint8_t>& Packet){
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadread_write_word_port(VTop *T){
std::vector<uint8_t> d;
// less than 32 bit
uint8_t tmp = 0;
tmp = (T->read_write_word_port >> 0) & 255;
d.push_back(tmp);
tmp = (T->read_write_word_port >> 8) & 255;
d.push_back(tmp);
tmp = (T->read_write_word_port >> 16) & 255;
d.push_back(tmp);
tmp = (T->read_write_word_port >> 24) & 255;
d.push_back(tmp);
return d;
}
void VerilatorSSTDUT::DirectWriteread_write_quad_port(VTop *T, const std::vector<uint8_t>& Packet){
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadread_write_quad_port(VTop *T){
std::vector<uint8_t> d;
// wider than 64 bits
uint8_t tmp = 0;
for (int i=0; i<5; i++) {
tmp = (T->read_write_quad_port[i] >> 0) & 255;
d.push_back(tmp);
tmp = (T->read_write_quad_port[i] >> 8) & 255;
d.push_back(tmp);
tmp = (T->read_write_quad_port[i] >> 16) & 255;
d.push_back(tmp);
tmp = (T->read_write_quad_port[i] >> 24) & 255;
d.push_back(tmp);
}
return d;
}
void VerilatorSSTDUT::DirectWriteread_write_double_port(VTop *T, const std::vector<uint8_t>& Packet){
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadread_write_double_port(VTop *T){
std::vector<uint8_t> d;
// less than 64 bit
uint8_t tmp = 0;
tmp = (T->read_write_double_port >> 0) & 255;
d.push_back(tmp);
tmp = (T->read_write_double_port >> 8) & 255;
d.push_back(tmp);
tmp = (T->read_write_double_port >> 16) & 255;
d.push_back(tmp);
tmp = (T->read_write_double_port >> 24) & 255;
d.push_back(tmp);
tmp = (T->read_write_double_port >> 32) & 255;
d.push_back(tmp);
tmp = (T->read_write_double_port >> 40) & 255;
d.push_back(tmp);
tmp = (T->read_write_double_port >> 48) & 255;
d.push_back(tmp);
tmp = (T->read_write_double_port >> 56) & 255;
d.push_back(tmp);
return d;
}
void VerilatorSSTDUT::DirectWriteread_write_array_port(VTop *T, const std::vector<uint8_t>& Packet){
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadread_write_array_port(VTop *T){
std::vector<uint8_t> d;
// less than 8 bit
for (int i=0; i<128; i++) {
d.push_back(T->read_write_array_port[i]);
}
return d;
}
void VerilatorSSTDUT::DirectWriteread_write_quad_array_port(VTop *T, const std::vector<uint8_t>& Packet){
}
std::vector<uint8_t> VerilatorSSTDUT::DirectReadread_write_quad_array_port(VTop *T){
std::vector<uint8_t> d;
// wider than 64 bits
for (int j=0; j<128; j++) {
uint8_t tmp = 0;
for (int i=0; i<4; i++) {
tmp = (T->read_write_quad_array_port[j][i] >> 0) & 255;
d.push_back(tmp);
tmp = (T->read_write_quad_array_port[j][i] >> 8) & 255;
d.push_back(tmp);
tmp = (T->read_write_quad_array_port[j][i] >> 16) & 255;
d.push_back(tmp);
tmp = (T->read_write_quad_array_port[j][i] >> 24) & 255;
d.push_back(tmp);
}
}
return d;
}

void VerilatorSSTDUT::handle_write_port(SST::Event* ev){
PortEvent *p = static_cast<PortEvent*>(ev);
// handle the message
if( p->getAtTick() != 0x00ull ){
writePortAtTick("write_port",p->getPacket(),p->getAtTick());
}else{
writePort("write_port",p->getPacket());
}
delete ev;
}
void VerilatorSSTDUT::handle_write_byte_port(SST::Event* ev){
PortEvent *p = static_cast<PortEvent*>(ev);
// handle the message
if( p->getAtTick() != 0x00ull ){
writePortAtTick("write_byte_port",p->getPacket(),p->getAtTick());
}else{
writePort("write_byte_port",p->getPacket());
}
delete ev;
}
void VerilatorSSTDUT::handle_write_reg_port(SST::Event* ev){
PortEvent *p = static_cast<PortEvent*>(ev);
// handle the message
if( p->getAtTick() != 0x00ull ){
writePortAtTick("write_reg_port",p->getPacket(),p->getAtTick());
}else{
writePort("write_reg_port",p->getPacket());
}
delete ev;
}
void VerilatorSSTDUT::handle_write_wire_port(SST::Event* ev){
PortEvent *p = static_cast<PortEvent*>(ev);
// handle the message
if( p->getAtTick() != 0x00ull ){
writePortAtTick("write_wire_port",p->getPacket(),p->getAtTick());
}else{
writePort("write_wire_port",p->getPacket());
}
delete ev;
}
void VerilatorSSTDUT::handle_write_half_port(SST::Event* ev){
PortEvent *p = static_cast<PortEvent*>(ev);
// handle the message
if( p->getAtTick() != 0x00ull ){
writePortAtTick("write_half_port",p->getPacket(),p->getAtTick());
}else{
writePort("write_half_port",p->getPacket());
}
delete ev;
}
void VerilatorSSTDUT::handle_write_word_port(SST::Event* ev){
PortEvent *p = static_cast<PortEvent*>(ev);
// handle the message
if( p->getAtTick() != 0x00ull ){
writePortAtTick("write_word_port",p->getPacket(),p->getAtTick());
}else{
writePort("write_word_port",p->getPacket());
}
delete ev;
}
void VerilatorSSTDUT::handle_write_quad_port(SST::Event* ev){
PortEvent *p = static_cast<PortEvent*>(ev);
// handle the message
if( p->getAtTick() != 0x00ull ){
writePortAtTick("write_quad_port",p->getPacket(),p->getAtTick());
}else{
writePort("write_quad_port",p->getPacket());
}
delete ev;
}
void VerilatorSSTDUT::handle_write_double_port(SST::Event* ev){
PortEvent *p = static_cast<PortEvent*>(ev);
// handle the message
if( p->getAtTick() != 0x00ull ){
writePortAtTick("write_double_port",p->getPacket(),p->getAtTick());
}else{
writePort("write_double_port",p->getPacket());
}
delete ev;
}
void VerilatorSSTDUT::handle_write_array_port(SST::Event* ev){
PortEvent *p = static_cast<PortEvent*>(ev);
// handle the message
if( p->getAtTick() != 0x00ull ){
writePortAtTick("write_array_port",p->getPacket(),p->getAtTick());
}else{
writePort("write_array_port",p->getPacket());
}
delete ev;
}
void VerilatorSSTDUT::handle_write_quad_array_port(SST::Event* ev){
PortEvent *p = static_cast<PortEvent*>(ev);
// handle the message
if( p->getAtTick() != 0x00ull ){
writePortAtTick("write_quad_array_port",p->getPacket(),p->getAtTick());
}else{
writePort("write_quad_array_port",p->getPacket());
}
delete ev;
}
void VerilatorSSTDUT::handle_read_write_port(SST::Event* ev){
// handle the message
std::vector<uint8_t> Packet;
Packet = readPort("read_write_port");
delete ev;
PortEvent *pe = new PortEvent(Packet);
}
void VerilatorSSTDUT::handle_read_write_byte_port(SST::Event* ev){
// handle the message
std::vector<uint8_t> Packet;
Packet = readPort("read_write_byte_port");
delete ev;
PortEvent *pe = new PortEvent(Packet);
}
void VerilatorSSTDUT::handle_read_write_reg_port(SST::Event* ev){
// handle the message
std::vector<uint8_t> Packet;
Packet = readPort("read_write_reg_port");
delete ev;
PortEvent *pe = new PortEvent(Packet);
}
void VerilatorSSTDUT::handle_read_write_wire_port(SST::Event* ev){
// handle the message
std::vector<uint8_t> Packet;
Packet = readPort("read_write_wire_port");
delete ev;
PortEvent *pe = new PortEvent(Packet);
}
void VerilatorSSTDUT::handle_read_write_half_port(SST::Event* ev){
// handle the message
std::vector<uint8_t> Packet;
Packet = readPort("read_write_half_port");
delete ev;
PortEvent *pe = new PortEvent(Packet);
}
void VerilatorSSTDUT::handle_read_write_word_port(SST::Event* ev){
// handle the message
std::vector<uint8_t> Packet;
Packet = readPort("read_write_word_port");
delete ev;
PortEvent *pe = new PortEvent(Packet);
}
void VerilatorSSTDUT::handle_read_write_quad_port(SST::Event* ev){
// handle the message
std::vector<uint8_t> Packet;
Packet = readPort("read_write_quad_port");
delete ev;
PortEvent *pe = new PortEvent(Packet);
}
void VerilatorSSTDUT::handle_read_write_double_port(SST::Event* ev){
// handle the message
std::vector<uint8_t> Packet;
Packet = readPort("read_write_double_port");
delete ev;
PortEvent *pe = new PortEvent(Packet);
}
void VerilatorSSTDUT::handle_read_write_array_port(SST::Event* ev){
// handle the message
std::vector<uint8_t> Packet;
Packet = readPort("read_write_array_port");
delete ev;
PortEvent *pe = new PortEvent(Packet);
}
void VerilatorSSTDUT::handle_read_write_quad_array_port(SST::Event* ev){
// handle the message
std::vector<uint8_t> Packet;
Packet = readPort("read_write_quad_array_port");
delete ev;
PortEvent *pe = new PortEvent(Packet);
}

// EOF
