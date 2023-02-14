//#pragma once
//
//// cmvs.h
//// 6/30/2015 jichi
//
//#include "engine/enginemodel.h"
//
//class kirikirizEngine : public EngineModel
//{
//  SK_EXTEND_CLASS(kirikirizEngine, EngineModel)
//  static bool attach();
//  static bool match(); 
//
//public:
//  kirikirizEngine()
//  {
//    name = "Embedkirikiriz";
//    //enableDynamicEncoding = true;
//    encoding = Utf16Encoding;
//    //newLineString = "\n";
//    matchFunction = &Self::match;
//    attachFunction = &Self::attach;
//  }
//};
//
//// EOF


#pragma once

// krkrz.h
// 6/15/2015 jichi

#include "engine/enginemodel.h"

class KiriKiriZEngine : public EngineModel
{
    SK_EXTEND_CLASS(KiriKiriZEngine, EngineModel)
        static bool attach(); static bool match(); 
public:
    KiriKiriZEngine()
    {
        name = "EmbedKiriKiriZ";
        encoding = Utf16Encoding;
        //matchFiles << "plugin/textrender.dll";
        attachFunction = &Self::attach;
        matchFunction = &Self::match; 
    }
};

// EOF
