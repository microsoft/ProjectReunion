#pragma once

#include "MainPage.g.h"

namespace winrt::$safeprojectname$::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage();
    };
}

namespace winrt::$safeprojectname$::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
