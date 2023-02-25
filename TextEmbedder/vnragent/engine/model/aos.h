  
#pragma once
 

#include "engine/enginemodel.h"

class aosEngine : public EngineModel
{
    SK_EXTEND_CLASS(aosEngine, EngineModel)
        static bool attach();  
public:
    aosEngine()
    {
        name = "AOSengine";
        enableDynamicEncoding = true;
        //encoding = Utf8Encoding;
        matchFiles<< "*.aos";
        attachFunction = &Self::attach; 
    }
};

// EOF
