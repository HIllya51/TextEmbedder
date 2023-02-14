#pragma once

// cmvs.h
// 6/30/2015 jichi

#include "engine/enginemodel.h"

class kirikirizEngine : public EngineModel
{
  SK_EXTEND_CLASS(kirikirizEngine, EngineModel)
  static bool attach();
  static bool match(); 

public:
  kirikirizEngine()
  {
    name = "Embedkirikiriz";
    enableDynamicEncoding = true;
    //newLineString = "\n";
    matchFunction = &Self::match;
    attachFunction = &Self::attach;
  }
};

// EOF
