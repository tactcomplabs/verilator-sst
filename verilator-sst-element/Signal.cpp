//
// _Signal_cpp_
//
// Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#include "Signal.h"

using namespace SST::VerilatorSST;

uint32_t Signal::calculateNumBytes(uint32_t nBits){
  assert(nBits > 0 && "nBits must be positive");
  const auto bytes = 1+((nBits-1)/(sizeof(uint8_t)*8));
  return bytes;
}

uint32_t Signal::calculateNumWords(uint32_t nBits){
  assert(nBits > 0 && "nBits must be positive");
  const auto words = 1+((nBits-1)/(sizeof(uint32_t)*8));
  return words;
}

Signal::Signal(uint32_t nBits, std::vector<uint8_t> initVal) : Signal(nBits,1,initVal,false){}

Signal::Signal(uint32_t nBits, uint64_t depth, const std::vector<uint8_t>& initVal, bool descending):
  depth(depth), 
  nBits(nBits){
  validate(nBits,depth);

  storage = new s_vpi_value[depth];

  const auto words = calculateNumWords(nBits);
  const auto bytes = calculateNumBytes(nBits);

  const auto buf = uint8ArrToUint32Arr(initVal,bytes,depth);
    
  for(auto i =0;i<depth;i++){
    p_vpi_value row = descending ? &storage[depth-i-1] : &storage[i];
    row->format = SIGNAL_VPI_FORMAT;
    row->value.vector = new s_vpi_vecval[words];
    for(int k=0;k<words;k++){
      row->value.vector[k].aval = buf[(i*words)+k];
      row->value.vector[k].bval = 0UL;
    }
  }
}

Signal::Signal(uint32_t nBits, uint64_t depth, p_vpi_value storage):
  nBits(nBits),
  depth(depth),
  storage(storage){
  validate(nBits,depth);
}

Signal::Signal(const Signal& other) : Signal(other.nBits, other.depth, other.getUIntVector(false), false){}

Signal::~Signal(){
  for(auto i=0;i<depth;i++){
    delete[] storage[i].value.vector;
  }
  delete[] storage;
}

void Signal::swap(Signal& first, Signal& second){
  std::swap(first.nBits, second.nBits);
  std::swap(first.depth, second.depth);
  std::swap(first.storage, second.storage);
}

Signal& Signal::operator=(Signal other){
  swap(*this, other);
  return *this;
}

uint32_t Signal::getNumBits() const{
  return nBits;
}

uint64_t Signal::getDepth() const{
  return depth;
}

s_vpi_value Signal::getVpiValue(uint64_t depth) const{
  assert(depth < this->depth && "depth out of range");
  s_vpi_value ret = storage[depth];

  return ret;
}

uint8_t Signal::getUIntScalar() const{
  uint8_t bit = storage[0].value.vector[0].aval & 1;
  return bit;
}

// currently unused
const std::vector<uint8_t> Signal::getUIntArray(uint64_t depth) const{
  assert(depth < this->depth && "depth out of range");

  const auto words = calculateNumWords(nBits);
  const auto bytes = calculateNumBytes(nBits);
  auto buf = std::vector<uint32_t>(words);
  for(auto i=0;i<words;i++){
    buf[i] = storage[depth].value.vector[i].aval;
  }

  const std::vector<uint8_t>& ret = uint32ArrToUint8Arr(buf,bytes,1);

  return ret;
}

const std::vector<uint8_t> Signal::getUIntVector(bool reverse) const{
  const auto words = calculateNumWords(nBits);
  const auto bytes = calculateNumBytes(nBits);

  auto buf = std::vector<uint32_t>(words*depth);
  for(auto i=0;i<depth;i++){
    auto row = reverse ? storage[depth-i-1] : storage[i];
    for(auto j=0;j<words;j++){
      buf[(i*words)+j] = row.value.vector[j].aval;
    }
  }

  const std::vector<uint8_t>& ret = uint32ArrToUint8Arr(buf, bytes, depth);

  return ret;
}

const std::vector<uint8_t> Signal::uint32ArrToUint8Arr(const std::vector<uint32_t>& src, const uint32_t bytesPerRow, const uint64_t rows){
  const auto wordsPerRow = calculateNumWords(bytesPerRow*8);
  auto buf = std::vector<uint8_t>(bytesPerRow*rows);
  for(auto i=0;i<rows;i++){
    auto rowBuf = std::vector<uint8_t>(bytesPerRow);
        
    for(auto j=0;j<wordsPerRow;j++){
      for(auto k=0;k<sizeof(uint32_t);k++){
        rowBuf[(j*sizeof(uint32_t))+k] = reinterpret_cast<const uint8_t*>(&src[(i*wordsPerRow)+j])[k];
      }
    }

    for(auto j=0;j<bytesPerRow;j++){
      buf[(i*bytesPerRow)+j] = rowBuf[j];
    }

  }

  return buf;
}

const std::vector<uint32_t> Signal::uint8ArrToUint32Arr(const std::vector<uint8_t>& src, const uint32_t bytesPerRow, const uint64_t rows){
  const auto wordsPerRow = calculateNumWords(bytesPerRow*8);
  auto buf = std::vector<uint32_t>(wordsPerRow*rows);

  for(auto i = 0; i < rows; i++){
    auto rowBuf = std::vector<uint32_t>(wordsPerRow);
    for(auto k = 0; k < bytesPerRow; k++){
      reinterpret_cast<uint8_t*>(rowBuf.data())[k] = src[(i*bytesPerRow)+k];
    }
    uint32_t mask = (bytesPerRow < 4) ? (1 << (bytesPerRow*8)) - 1 : 0xffffffff;
    for(auto j = 0; j < wordsPerRow; j++){
      buf[(i*wordsPerRow)+j] = rowBuf[j] & mask;
    }
  }

  return buf;
}

void Signal::validate(uint32_t nBits, uint64_t depth){
  assert(depth > 0 && "depth must be positive");
  assert(nBits > 0 && "nBits must be positive");
  assert(nBits <= SIGNAL_BITS_MAX && "nBits cannot be greater than SIGNAL_BITS_MAX");
}

//SIGNAL FACTORY

SignalFactory::SignalFactory(uint32_t nBits, uint64_t depth): 
bits(nBits),
words(Signal::calculateNumWords(bits)),
depth(depth){
  assert(depth > 0 && "depth must be positive");
  storage = new s_vpi_value[depth];
}

SignalFactory::SignalFactory() : SignalFactory(1,1){};
SignalFactory::~SignalFactory(){
  if(!storage){
    return;
  }

  for(auto i=0;i<depth;i++){
    delete[] storage[i].value.vector;
  }
  delete[] storage;
}

Signal * SignalFactory::operator()(const s_vpi_value &row){
  assert(nextRow < depth && "cannot add more rows");
  assert(row.format == SIGNAL_VPI_FORMAT && "all rows must be SIGNAL_VPI_FORMAT");

  storage[nextRow].format = SIGNAL_VPI_FORMAT;
  storage[nextRow].value.vector = new s_vpi_vecval[words];
  std::memcpy(storage[nextRow].value.vector, row.value.vector, words*sizeof(uint32_t)*2); // *2 to account for aval and bval
    
  nextRow++;  

  if(nextRow == depth){
    Signal * signal = new Signal(bits,depth,storage);
    storage = nullptr;

    return signal;
  }
  return nullptr;
}
