#include "Ouroboros/Scripting/ExportAPI.h"

#include "Ouroboros/UI/RectTransformComponent.h"

namespace oo
{
    SCRIPT_API_GET_SET_FUNC_VECTOR3(RectTransformComponent, AnchoredPosition, GetAnchoredPosition, SetAnchoredPosition)
    SCRIPT_API_GET_SET_FUNC_VECTOR3(RectTransformComponent, LocalEulerAngles, GetEulerAngles, SetEulerAngles)
    SCRIPT_API_GET_SET_FUNC_VECTOR3(RectTransformComponent, LocalScale, GetScale, SetScale)

    SCRIPT_API_GET_SET_FUNC_VECTOR2(RectTransformComponent, Size, GetSize, SetSize)
    SCRIPT_API_GET_SET_FUNC_VECTOR2(RectTransformComponent, Pivot, GetPivot, SetPivot)
    SCRIPT_API_GET_SET_FUNC_VECTOR2(RectTransformComponent, AnchorMin, GetAnchorMin, SetAnchorMin)
    SCRIPT_API_GET_SET_FUNC_VECTOR2(RectTransformComponent, AnchorMax, GetAnchorMax, SetAnchorMax)
}
