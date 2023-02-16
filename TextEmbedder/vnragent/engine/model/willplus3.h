  
#pragma once
 

#include "engine/enginemodel.h"

class willplus3Engine : public EngineModel
{
    SK_EXTEND_CLASS(willplus3Engine, EngineModel)
        static bool attach();  
public:
    willplus3Engine()
    {
        name = "Embedwillplus3";
        encoding = Utf16Encoding;
        matchFiles << "Rio.arc" << "Chip*.arc";
        attachFunction = &Self::attach; 
    }
};

// EOF
