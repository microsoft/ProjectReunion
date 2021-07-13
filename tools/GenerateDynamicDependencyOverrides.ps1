# Make a valid AppxManifest.xml from a templated file and variable substititon

Param(
    [Parameter(Mandatory)]
    [ValidateNotNullOrEmpty()]
    [string]$Path
)

function Convert-Guid
{
    Param([string]$uuid)

    $fields = $uuid -Split '-', 5
    $field0 = $fields[0]
    $field1 = $fields[1]
    $field2 = $fields[2]
    $field3 = $fields[3]
    $b0 = $field3.SubString(0, 2)
    $b1 = $field3.SubString(2, 2)
    $field4 = $fields[4]
    $b2 = $field4.SubString(0, 2)
    $b3 = $field4.SubString(2, 2)
    $b4 = $field4.SubString(4, 2)
    $b5 = $field4.SubString(6, 2)
    $b6 = $field4.SubString(8, 2)
    $b7 = $field4.SubString(10, 2)
    $guid = "0x$field0, 0x$field1, 0x$field2, { 0x$b0, 0x$b1, 0x$b2, 0x$b3, 0x$b4, 0x$b5, 0x$b6, 0x$b7 }"
    return $guid
}

if (-not(Test-Path -Path $Path -PathType Container))
{
    Write-Host "Creating $Path..."
    $override = New-Item -Path $Path -ItemType Directory -Force
}
else
{
    Write-Output "$Path exists"
}

$lifetimemanager_clsid_uuid = New-Guid
$lifetimemanager_clsid_guid = Convert-Guid $lifetimemanager_clsid_uuid

$pushnotifications_clsid_uuid = New-Guid

# Generate the json file
$content_json=@"
{
    "LIBID": "1BF712E5-41ED-46a5-8402-2D40725F691E",
    "DataStore": {
        "CLSID": {
            "UUID": "D1AD16C7-EC59-4765-BF95-9A243EB00507"
        }
    },
    "PushNotifications": {
        "LIBID": "CE96C745-3017-460E-895B-4FD98E1194F2",
        "Impl": {
            "CLSID": {
                "UUID": "E739C755-0D09-48DF-A468-A5DF0B5422DC"
            }
        },
        "Task": {
            "CLSID": {
                "UUID": "$pushnotifications_clsid_uuid"
            }
        }
    },
    "LifetimeManager": {
        "CLSID": {
            "UUID": "$lifetimemanager_clsid_uuid",
            "GUID": "$lifetimemanager_clsid_guid"
        }
    }
}
"@
$file_json = Join-Path $Path 'DynamicDependency-Override.json'
Write-Output "Writing $file_json..."
"$content_json" | Out-File $file_json -Encoding utf8

# Generate the Dynamic Dependency header file
$content_h=@"
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

// Rely on _STRINGIZE(x) in yvals_core.h
#ifndef _STRINGIZE
#define _STRINGIZEX(x)  #x
#define _STRINGIZE(x)   _STRINGIZEX(x)
#endif

#ifdef PR_DYNDEP_LIBID_UUID
#undef PR_DYNDEP_LIBID_UUID
#define PR_DYNDEP_LIBID_UUID                        1BF712E5-41ED-46a5-8402-2D40725F691E
#endif

#ifdef PR_DYNDEP_DATASTORE_CLSID_UUID
#undef PR_DYNDEP_DATASTORE_CLSID_UUID
#define PR_DYNDEP_DATASTORE_CLSID_UUID              D1AD16C7-EC59-4765-BF95-9A243EB00507
#endif

#ifdef PR_DYNDEP_DATASTORE_CLSID_STRING
#undef PR_DYNDEP_DATASTORE_CLSID_STRING
#define PR_DYNDEP_DATASTORE_CLSID_STRING            _STRINGIZE(PR_DYNDEP_DATASTORE_CLSID_UUID)
#endif

#ifdef PR_DYNDEP_LIFETIMEMANAGER_CLSID_UUID
#undef PR_DYNDEP_LIFETIMEMANAGER_CLSID_UUID
#define PR_DYNDEP_LIFETIMEMANAGER_CLSID_UUID        $lifetimemanager_clsid_uuid
#endif

#ifdef PR_DYNDEP_LIFETIMEMANAGER_CLSID_STRING
#undef PR_DYNDEP_LIFETIMEMANAGER_CLSID_STRING
#define PR_DYNDEP_LIFETIMEMANAGER_CLSID_STRING      _STRINGIZE(PR_DYNDEP_LIFETIMEMANAGER_CLSID_UUID)
#endif

#ifdef PR_DYNDEP_LIFETIMEMANAGER_CLSID_GUID
#undef PR_DYNDEP_LIFETIMEMANAGER_CLSID_GUID
#define PR_DYNDEP_LIFETIMEMANAGER_CLSID_GUID        $lifetimemanager_clsid_guid
#endif
"@
$file_h = Join-Path $Path 'DynamicDependency-Override.h'
Write-Output "Writing $file_h..."
"$content_h" | Out-File $file_h -Encoding utf8

# Generate the Push Notifications header file
$content_h=@"
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

// Rely on _STRINGIZE(x) in yvals_core.h
#ifndef _STRINGIZE
#define _STRINGIZEX(x)  #x
#define _STRINGIZE(x)   _STRINGIZEX(x)
#endif

#ifdef PUSHNOTIFICATIONS_LIBID_UUID
#undef PUSHNOTIFICATIONS_LIBID_UUID
#define PUSHNOTIFICATIONS_LIBID_UUID             CE96C745-3017-460E-895B-4FD98E1194F2
#endif

#ifdef PUSHNOTIFICATIONS_IMPL_CLSID_UUID
#undef PUSHNOTIFICATIONS_IMPL_CLSID_UUID
#define PUSHNOTIFICATIONS_IMPL_CLSID_UUID        E739C755-0D09-48DF-A468-A5DF0B5422DC
#endif

#ifdef PUSHNOTIFICATIONS_IMPL_CLSID_STRING
#undef PUSHNOTIFICATIONS_IMPL_CLSID_STRING
#define PUSHNOTIFICATIONS_IMPL_CLSID_STRING      _STRINGIZE(PUSHNOTIFICATIONS_IMPL_CLSID_UUID)
#endif

#ifdef PUSHNOTIFICATIONS_TASK_CLSID_UUID
#undef PUSHNOTIFICATIONS_TASK_CLSID_UUID
#define PUSHNOTIFICATIONS_TASK_CLSID_UUID        $pushnotifications_clsid_uuid
#endif

#ifdef PUSHNOTIFICATIONS_TASK_CLSID_STRING
#undef PUSHNOTIFICATIONS_TASK_CLSID_STRING
#define PUSHNOTIFICATIONS_TASK_CLSID_STRING      _STRINGIZE(PUSHNOTIFICATIONS_TASK_CLSID_UUID)
#endif
"@
$file_h = Join-Path $Path 'PushNotifications-Override.h'
Write-Output "Writing $file_h..."
"$content_h" | Out-File $file_h -Encoding utf8 
