// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "pch.h"
#include "winrt/Microsoft.Windows.System.Power.h"

using namespace std::chrono_literals;
using namespace winrt::Microsoft::Windows::System::Power;

namespace Test::PowerNotifications
{
    // Timeout in milliseconds
    constexpr auto c_timeoutInMSec{ 2000 };
    class APITests
    {
    public:
        BEGIN_TEST_CLASS(APITests)
            TEST_CLASS_PROPERTY(L"ThreadingModel", L"MTA")
            TEST_CLASS_PROPERTY(L"RunFixtureAs:Class", L"RestrictedUser")
            TEST_CLASS_PROPERTY(L"RunAs", L"UAP")
            TEST_CLASS_PROPERTY(L"UAP:AppxManifest", L"PowerNotifications-AppxManifest.xml")
        END_TEST_CLASS()

        TEST_CLASS_SETUP(ClassInit)
        {
            ::Test::Bootstrap::SetupPackages();
            return true;
        }

        TEST_CLASS_CLEANUP(ClassUninit)
        {
            ::Test::Bootstrap::CleanupPackages();
            return true;
        }

        TEST_METHOD_SETUP(MethodInit)
        {
            VERIFY_IS_TRUE(TP::IsPackageRegistered_ProjectReunionFramework());
            VERIFY_IS_TRUE(TP::IsPackageRegistered_DynamicDependencyDataStore());
            VERIFY_IS_TRUE(TP::IsPackageRegistered_DynamicDependencyLifetimeManager());
            return true;
        }

        TEST_METHOD_CLEANUP(MethodUninit)
        {
            VERIFY_IS_TRUE(TP::IsPackageRegistered_ProjectReunionFramework());
            VERIFY_IS_TRUE(TP::IsPackageRegistered_DynamicDependencyDataStore());
            VERIFY_IS_TRUE(TP::IsPackageRegistered_DynamicDependencyLifetimeManager());
            return true;
        }

        TEST_METHOD(GetBatteryStatus)
        {
            auto value = PowerManager::BatteryStatus();
            VERIFY_ARE_EQUAL(value, BatteryStatus::NotPresent);
        }

        TEST_METHOD(GetPowerSupplyStatus)
        {
            auto value = PowerManager::PowerSupplyStatus();
            VERIFY_ARE_EQUAL(value, PowerSupplyStatus::Adequate);
        }

        TEST_METHOD(GetRemainingChargePercent)
        {
            auto value = PowerManager::RemainingChargePercent();
            VERIFY_ARE_EQUAL(value, 100);
        }

        TEST_METHOD(CompositeBatteryStatusCallback)
        {
            // This test can only be run on systems/VM with a battery
            // The values will need to be changed to reflect real-time values
            // when running the test on such systems
            BEGIN_TEST_METHOD_PROPERTIES()
                TEST_METHOD_PROPERTY(L"Ignore", L"true")                
            END_TEST_METHOD_PROPERTIES()

            wil::unique_handle event(CreateEvent(nullptr, false, false, nullptr));
            THROW_LAST_ERROR_IF_NULL(event.get());
            auto batteryStat = BatteryStatus::Discharging;
            auto powerStat = PowerSupplyStatus::Inadequate;
            int32_t remainingCharge = 15;
            auto token = PowerManager::BatteryStatusChanged([&](const auto&, winrt::Windows::Foundation::IInspectable obj)
                {
                    batteryStat = PowerManager::BatteryStatus();
                    powerStat = PowerManager::PowerSupplyStatus();
                    remainingCharge = PowerManager::RemainingChargePercent();
                    SetEvent(event.get());
                });

            VERIFY_IS_TRUE(WaitForSingleObject(event.get(), c_timeoutInMSec) == WAIT_OBJECT_0);
            VERIFY_ARE_EQUAL(batteryStat, BatteryStatus::Idle);
            VERIFY_ARE_EQUAL(powerStat, PowerSupplyStatus::Adequate);
            VERIFY_ARE_EQUAL(remainingCharge, 100);
            PowerManager::BatteryStatusChanged(token);
        }

        TEST_METHOD(GetRemainingDischargeTime)
        {
            auto value = PowerManager::RemainingDischargeTime();
            VERIFY_ARE_EQUAL(value.count(), -10000000);
        }

        TEST_METHOD(RemainingDischargeTimeCallback)
        {
            static wil::unique_handle event(CreateEvent(nullptr, false, false, nullptr));
            THROW_LAST_ERROR_IF_NULL(event.get());

            auto value = winrt::Windows::Foundation::TimeSpan(1s);
            auto token = PowerManager::RemainingDischargeTimeChanged([&](const auto&, winrt::Windows::Foundation::IInspectable obj)
                {
                    value = PowerManager::RemainingDischargeTime();
                    SetEvent(event.get());
                });
            VERIFY_IS_TRUE(WaitForSingleObject(event.get(), c_timeoutInMSec) == WAIT_OBJECT_0);
            VERIFY_ARE_EQUAL(value.count(), -10000000);
            PowerManager::RemainingDischargeTimeChanged(token);
        }

        TEST_METHOD(GetEnergySaverStatus)
        {
            auto value = PowerManager::EnergySaverStatus();
            VERIFY_ARE_EQUAL(value, EnergySaverStatus::Disabled);
        }

        TEST_METHOD(EnergySaverStatusCallback)
        {
            wil::unique_handle event(CreateEvent(nullptr, false, false, nullptr));
            THROW_LAST_ERROR_IF_NULL(event.get());
            auto value = EnergySaverStatus::Disabled;
            auto token = PowerManager::EnergySaverStatusChanged([&](const auto&, winrt::Windows::Foundation::IInspectable obj)
                {
                    value = PowerManager::EnergySaverStatus();
                    SetEvent(event.get());
                });

            VERIFY_IS_TRUE(WaitForSingleObject(event.get(), c_timeoutInMSec) == WAIT_OBJECT_0);
            VERIFY_ARE_EQUAL(value, EnergySaverStatus::Disabled);
            PowerManager::EnergySaverStatusChanged(token);
        }

        TEST_METHOD(GetPowerSourceKind)
        {
            auto value = PowerManager::PowerSourceKind();
            VERIFY_ARE_EQUAL(value, PowerSourceKind::AC);
        }

        TEST_METHOD(PowerSourceKindCallback)
        {
            wil::unique_handle event(CreateEvent(nullptr, false, false, nullptr));
            THROW_LAST_ERROR_IF_NULL(event.get());
            auto value = PowerSourceKind::DC;
            auto token = PowerManager::PowerSourceKindChanged([&](const auto&, winrt::Windows::Foundation::IInspectable obj)
                {
                    value = PowerManager::PowerSourceKind();
                    SetEvent(event.get());
                });

            VERIFY_IS_TRUE(WaitForSingleObject(event.get(), c_timeoutInMSec) == WAIT_OBJECT_0);
            VERIFY_ARE_EQUAL(value, PowerSourceKind::AC);
            PowerManager::PowerSourceKindChanged(token);
        }

        TEST_METHOD(GetDisplayStatus)
        {
            auto value = PowerManager::DisplayStatus();
            VERIFY_ARE_EQUAL(value, DisplayStatus::On);
        }

        TEST_METHOD(DisplayStatusCallback)
        {
            wil::unique_handle event(CreateEvent(nullptr, false, false, nullptr));
            THROW_LAST_ERROR_IF_NULL(event.get());
            auto value = DisplayStatus::Off;
            auto token = PowerManager::DisplayStatusChanged([&](const auto&, winrt::Windows::Foundation::IInspectable obj)
                {
                    value = PowerManager::DisplayStatus();
                    SetEvent(event.get());
                });

            VERIFY_IS_TRUE(WaitForSingleObject(event.get(), c_timeoutInMSec) == WAIT_OBJECT_0);
            VERIFY_ARE_EQUAL(value, DisplayStatus::On);
            PowerManager::DisplayStatusChanged(token);
        }

        TEST_METHOD(SystemIdleStatusCallback)
        {
            // Ignoring this test since there is no default value for SystemIdle
            // and there is no way to know when the callback will be fired
            BEGIN_TEST_METHOD_PROPERTIES()
                TEST_METHOD_PROPERTY(L"Ignore", L"true")                
            END_TEST_METHOD_PROPERTIES()

            auto callback_success = false;
            auto token = PowerManager::SystemIdleStatusChanged([&](const auto&, winrt::Windows::Foundation::IInspectable obj)
                {
                    callback_success = true;
                });
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            PowerManager::SystemIdleStatusChanged(token);
            VERIFY_ARE_EQUAL(callback_success, true);
        }

        TEST_METHOD(GetUserPresenceStatus)
        {
            auto value = PowerManager::UserPresenceStatus();
            VERIFY_ARE_EQUAL(value, UserPresenceStatus::Present);
        }

        TEST_METHOD(UserPresenceStatusCallback)
        {
            wil::unique_handle event(CreateEvent(nullptr, false, false, nullptr));
            THROW_LAST_ERROR_IF_NULL(event.get());
            auto value = UserPresenceStatus::Absent;
            auto token = PowerManager::UserPresenceStatusChanged([&](const auto&, winrt::Windows::Foundation::IInspectable obj)
                {
                    value = PowerManager::UserPresenceStatus();
                    SetEvent(event.get());
                });

            VERIFY_IS_TRUE(WaitForSingleObject(event.get(), c_timeoutInMSec) == WAIT_OBJECT_0);
            VERIFY_ARE_EQUAL(value, UserPresenceStatus::Present);
            PowerManager::UserPresenceStatusChanged(token);
        }

        TEST_METHOD(SystemSuspendStatusCallback)
        {
            // Since the called API is for sleep/hibernate, we just test the registration
            auto token = PowerManager::SystemSuspendStatusChanged([&](const auto&, winrt::Windows::Foundation::IInspectable) {});
            VERIFY_IS_NOT_NULL(token);
            PowerManager::SystemSuspendStatusChanged(token);
        }
    };
}
