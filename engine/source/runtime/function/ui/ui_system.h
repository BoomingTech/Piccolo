#pragma once

#include "runtime/core/base/public_singleton.h"

namespace Pilot
{
    class PUIManager final : public PublicSingleton<PUIManager>
    {
        friend class PublicSingleton<PUIManager>;

    public:
        PUIManager(const PUIManager&) = delete;
        PUIManager& operator=(const PUIManager&) = delete;
        //
    protected:
        PUIManager() = default;

    public:
        int initialize();
        int update();
        int clear();
    };
} // namespace Pilot
