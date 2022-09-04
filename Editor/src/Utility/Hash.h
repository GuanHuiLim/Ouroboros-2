/************************************************************************************//*!
\file           Hash.h
\project        Ouroboros
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           June 14, 2022
\brief          Contains a utility hashing class that stores the various hashing algorithms
                currently only supports fnv-1a hash.


Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <string>

#include <compare>

/********************************************************************************//*!
@brief Contains a utility hashing class that stores the various hashing algorithms
        currently only supports fnv-1a hash.
*//*********************************************************************************/
struct StringHash
{
public:
    using size_type = std::uint32_t;

    constexpr static size_t const_strlen(char const* s);
    static const size_type  GenerateFNV1aHash(char const* str);
    static const size_type GenerateFNV1aHash(std::string_view string);

private:
    size_type computedHash;

public:
    StringHash(StringHash const& other) = default;

    StringHash(size_type hash) noexcept;
    StringHash(char const* s) noexcept;
    StringHash(std::string_view s) noexcept;
    StringHash(std::string const& s) noexcept;

    operator size_type() const noexcept;
    //constexpr bool operator==(StringHash const& other) const noexcept;
    //constexpr bool operator<(StringHash const& other) const noexcept;// { return computedHash < other.computedHash; }

    auto operator<=>(StringHash const& other) const = default;
};

//// hashing overload for StringHash with std lib
//namespace std
//{
//    template<>
//    struct hash<StringHash>
//    {
//        std::size_t operator() (StringHash const& newhash) const
//        {
//            return hash<std::size_t>()((std::uint32_t)newhash);
//        }
//    };
//}
