#pragma once
#include "PackagingProgress.g.h"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    struct PackagingProgress : PackagingProgressT<PackagingProgress>
    {
        PackagingProgress() = default;

        double Percentage();
        void Percentage(double);

        double mPercentage{};
    };
}

namespace winrt::Microsoft::Kozani::MakeMSIX::factory_implementation
{
    struct PackagingProgress : PackagingProgressT<PackagingProgress, implementation::PackagingProgress>
    {
    };
}
#pragma once
