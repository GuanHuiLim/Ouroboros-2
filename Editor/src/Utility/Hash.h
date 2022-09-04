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
    static constexpr size_type  GenerateFNV1aHash(const char* str)
    {
        // Also C++ does not like static constexpr
        constexpr size_type FNV_PRIME = 16777619u;
        constexpr size_type OFFSET_BASIS = 2166136261u;

        const size_t length = const_strlen(str) + 1;
        size_type hash = OFFSET_BASIS;
        for (size_t i = 0; i < length; ++i)
        {
            hash ^= *str++;
            hash *= FNV_PRIME;
        }
        return hash;
    }
    static constexpr size_type GenerateFNV1aHash(std::string_view string)
    {
        return GenerateFNV1aHash(string.data());
    }
    static constexpr size_t const_strlen(const char* s)
    {
        size_t size = 0;
        while (s[size]) { size++; };
        return size;
    }

private:
    size_type computedHash;

public:
    StringHash(StringHash const& other) = default;

    //constexpr StringHash(size_type hash) noexcept;
    //constexpr StringHash(char const* s) noexcept;
    //constexpr StringHash(std::string_view s) noexcept;
    //constexpr StringHash(std::string const& s) noexcept;

    //constexpr operator size_type() const noexcept;


    constexpr StringHash(size_type hash) noexcept
        : computedHash(hash)
    {
    }

    constexpr StringHash(const char* s) noexcept
        : computedHash(0)
    {
        computedHash = GenerateFNV1aHash(s);
    }

    constexpr StringHash(std::string_view s) noexcept
        : computedHash(0)
    {
        computedHash = GenerateFNV1aHash(s.data());
    }

    constexpr StringHash(std::string const& s) noexcept
        : computedHash(0)
    {
        computedHash = GenerateFNV1aHash(s.data());
    }

    constexpr operator size_type() const noexcept
    {
        return computedHash;
    }


    //constexpr bool operator==(StringHash const& other) const noexcept;
    //constexpr bool operator<(StringHash const& other) const noexcept;// { return computedHash < other.computedHash; }

    auto operator<=>(StringHash const& other) const = default;
};

// hashing overload for StringHash with std lib
namespace std
{
    template<>
    struct hash<StringHash>
    {
        std::size_t operator() (StringHash const& newhash) const
        {
            return hash<std::size_t>()((std::uint32_t)newhash);
        }
    };
}
