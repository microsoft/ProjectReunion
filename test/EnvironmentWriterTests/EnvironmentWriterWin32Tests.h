﻿// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#include "TestSetupAndTeardownHelper.h"

namespace ProjectReunionEnvironmentWriterTests
{
    class EnvironmentWriterWin32Tests {
        BEGIN_TEST_CLASS(EnvironmentWriterWin32Tests)
            TEST_CLASS_PROPERTY(L"ActivationContext", L"EnvironmentWriterTests.dll.manifest")
            TEST_CLASS_PROPERTY(L"ThreadingModel", L"MTA")
            //TEST_CLASS_PROPERTY(L"RunAs", L"{RestrictedUser,ElevatedUser,InteractiveUser}")
            END_TEST_CLASS()

        TEST_METHOD(TestGetForProcess);
        TEST_METHOD(TestGetForUser);
        TEST_METHOD(TestGetForMachine);
        TEST_METHOD(TestSetEnvironmentVariableForProcess);
        TEST_METHOD(TestSetEnvironmentVariableForUser);
        TEST_METHOD(TestSetEnvironmentVariableForMachine);
        TEST_METHOD(TestAppendToPathForProcess);
        TEST_METHOD(TestAppendToPathForUser);
        TEST_METHOD(TestAppendToPathForMachine);
        TEST_METHOD(TestRemoveFromPathForProcess);
        TEST_METHOD(TestRemoveFromPathForUser);
        TEST_METHOD(TestRemoveFromPathForMachine);
    };
}
