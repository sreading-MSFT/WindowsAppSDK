#include "pch.h"
#include "PackagingProgress.h"
#include "PackagingProgress.g.cpp"

namespace winrt::Microsoft::Kozani::MakeMSIX::implementation
{
    double PackagingProgress::Percentage()
    {
        return mPercentage;
    }
    void PackagingProgress::Percentage(double value)
    {
        mPercentage = value;
    }
}
