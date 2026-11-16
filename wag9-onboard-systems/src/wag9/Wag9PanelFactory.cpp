#include "Wag9PanelFactory.h"

namespace Wag9
{
    std::unique_ptr<Wag9FsInfoPanel> PanelFactory::create(Wag9Zone) const
    {
        return nullptr;
    }
}