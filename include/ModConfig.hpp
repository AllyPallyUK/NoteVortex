#pragma once

#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(ModConfig,
    CONFIG_VALUE(modEnabled, bool, "Mod enabled", false);
    CONFIG_VALUE(worldRotationOn, bool, "Enable world rotation", false);
    CONFIG_VALUE(changeStartPos, bool, "Enable note effect", false);
    CONFIG_VALUE(newUniformScale, float, "Note size multiplier", 1.0f)

    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(modEnabled);
        CONFIG_INIT_VALUE(worldRotationOn);
        CONFIG_INIT_VALUE(changeStartPos);
        CONFIG_INIT_VALUE(newUniformScale);
    )
);