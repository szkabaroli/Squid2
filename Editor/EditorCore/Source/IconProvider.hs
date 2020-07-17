#pragma once
#include <Core/Types.h>

namespace Squid {

namespace EditorCore {

    enum class EditorIcon { SAVE_ICON, NEW_ICON, ENTITY_ICON, MOVE_ICON, ROTATE_ICON, SCALE_ICON, NONE_ICON };

    static constexpr u16 ICONS_TEXTURE_SIZE = 512;
    static constexpr u16 ICON_SIZE = 32;
    static constexpr u16 ICON_COUNT_PER_ROW = ICONS_TEXTURE_SIZE / ICON_SIZE;

    static constexpr f32 UV_OFFSET = 1.0 / (float)ICON_COUNT_PER_ROW;

    class IconProvider {};

} // namespace EditorCore
} // namespace Squid
