#pragma once

#define REUNION_SDK_VER 1.0
#define COMPONENT_NAME "Reunion telemetry sample"

#define PartB_Reunion() \
        TraceLoggingStruct(2, "PartB_Reunion_API"), \
        TraceLoggingFloat32(REUNION_SDK_VER, "API version"), \
        TraceLoggingString(COMPONENT_NAME, "ComponentName")

