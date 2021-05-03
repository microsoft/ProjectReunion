#pragma once

#include "resource.h"

class TelemetryTestOSSSample : public wil::TraceLoggingProvider
{
    IMPLEMENT_TRACELOGGING_CLASS_WITH_MICROSOFT_TELEMETRY(TelemetryTestOSSSample, "Microsoft.Reunion.TelemetryTestOSS.Provider",
        // {eb27a80d-3bf8-5038-3b4d-04fab367b574}
        (0xeb27a80d, 0x3bf8, 0x5038, 0x3b, 0x4d, 0x04, 0xfa, 0xb3, 0x67, 0xb5, 0x74));
public:
    DEFINE_COMPLIANT_TELEMETRY_EVENT_BOOL(BoolEvent, PDT_ProductAndServiceUsage, boolValue);
    DEFINE_COMPLIANT_TELEMETRY_EVENT_STRING(StringEvent, PDT_ProductAndServiceUsage, stringValue);
};
