﻿// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#include "ResourceLoader.g.h"

namespace winrt::Microsoft::WindowsApp::Resources::implementation
{

struct ResourceLoader : ResourceLoaderT<ResourceLoader>
{
    ResourceLoader();
    ResourceLoader(hstring const& fileName);
    ResourceLoader(hstring const& fileName, hstring const& resourceMap);

    static hstring GetDefaultResourceFilePath();

    hstring GetString(hstring const& resourceId);
    hstring GetStringForUri(Windows::Foundation::Uri const& resourceUri);

private:
    ~ResourceLoader();

    MrmManagerHandle m_resourceManager = nullptr;
    MrmMapHandle m_currentResourceMap = nullptr;
};

} // namespace winrt::Microsoft::WindowsApp::Resources::implementation

namespace winrt::Microsoft::WindowsApp::Resources::factory_implementation
{

struct ResourceLoader : ResourceLoaderT<ResourceLoader, implementation::ResourceLoader>
{};

} // namespace winrt::Microsoft::WindowsApp::Resources::factory_implementation
