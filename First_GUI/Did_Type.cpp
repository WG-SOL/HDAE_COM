#include "Did_Type.h"
const DID_Table_t didTable[] = {
    {0x1001,
     2,
     "ToF_Sensor_Distance",
     0,
     0
    },
    {0x1002,
     2,
     "LightSensor",
     0,
     0
    },
    {0x2001,
     2,
     "ABEThreshold",
     0,
     0
    },
    {0x2002,
     2,
     "HeadlightThreshold",
     0,
     0
    },
    {0x4000,
     2,
     "MotorControl",
     0,
     0
    },
    {0x3000,
     2,
     "HeadlightControl",
     0,
     0
    }
    // 필요하면 DID 추가
};

const int DID_TABLE_SIZE = sizeof(didTable)/sizeof(didTable[0]);
