#pragma once

#include "Wag9FsInfoPanel.h"

#include <memory>

namespace Wag9
{
    class IPanelFactory
    {
        public:
            virtual ~IPanelFactory() = default;
            virtual std::unique_ptr<Wag9FsInfoPanel> create(Wag9Zone zone) const = 0;
    };

    class PanelFactory : public IPanelFactory
    {
        public:
            virtual std::unique_ptr<Wag9FsInfoPanel> create(Wag9Zone zone) const override;
    };
}