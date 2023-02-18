#pragma once
#include "MathCommon.h"
#include "Geometry.h"

#include <string>
#include <map>

namespace oGFX {
    

enum class FontType : uint8_t
{
    Bitmap = 0,
    SDF = 1,
};

struct FontProperties
{
    FontType type = FontType::Bitmap;
    glm::uvec2 faceSize{ 0,72 };
};

enum class FontAlignment : int
{
    Top_Left      = 1 << 0,
    Top_Right     = 1 << 1,
    Top_Centre    = 1 << 2,
    Centre_Left   = 1 << 3,
    Centre        = 1 << 4,
    Centre_Right  = 1 << 5,
    Bottom_Left   = 1 << 6,
    Bottom_Right  = 1 << 7,
    Bottom_Centre = 1 << 8,
};    
// return uint32_t as non-explicit underlying type for enum class is undefined
inline int32_t operator|(oGFX::FontAlignment lhs, oGFX::FontAlignment rhs) { return (static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs)); }
inline int32_t operator|(oGFX::FontAlignment lhs, int32_t rhs) { return (static_cast<int32_t>(lhs) | rhs); }
inline int32_t operator|(int32_t lhs, oGFX::FontAlignment  rhs) { return (rhs | lhs); }
inline int32_t operator&(oGFX::FontAlignment lhs, oGFX::FontAlignment rhs) { return (static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs)); }
inline int32_t operator&(oGFX::FontAlignment lhs, int32_t rhs) { return (static_cast<int32_t>(lhs) & rhs); }
inline int32_t operator&(int32_t lhs, oGFX::FontAlignment  rhs) { return (rhs & lhs); }

struct FontFormatting
{
    float verticalLineSpace{ 1.0f };
    float fontSize{ 72.0f };
    oGFX::AABB2D box{ {},{} };
    FontAlignment alignment{ FontAlignment::Centre };
};

class Font
{
public:
    struct Glyph
    {
        uint32_t textureIndex;  // index in which the texture resides
        glm::vec4 textureCoordinates;  // tex coords of the glyph in the atlas
        glm::vec2   Size;       // Size of glyph
        glm::vec2   Bearing;    // Offset from baseline to left/top of glyph
        glm::vec2   Advance;    // Offset to advance to next glyph
    };

    virtual void* Get_IMTEXTURE_ID() const { return reinterpret_cast<void*>(static_cast<uint64_t>(m_atlasID)); }

public:
    // this is probably bad af
    std::wstring m_name;
    using wideChar = std::wstring::value_type;
    std::map<wideChar, Glyph> m_characterInfos;

    uint32_t m_atlasID{ 0 };
    uint32_t m_pixelSize{ 72 };

    bool m_loaded{ false };

    //Image2D m_fontAtlas;
};

}// end namespace oGFX
