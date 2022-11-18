/************************************************************************************//*!
\file           uuid.h
\project        Physics
\author         Chua Teck Lee, c.tecklee, 390008420 | code contribution (100%)
\par            email: c.tecklee\@digipen.edu
\date           Jun 09, 2022
\brief          File describing a Universally Unique Identifier
                "UUID" (universally unique identifier) or GUID is (usually) a 128-bit integer
                used to "uniquely" identify information.
                What is a GUID? http://guid.one/guid
                Mordern Solution to UUID with standard library :
                https://stackoverflow.com/questions/24365331/how-can-i-generate-uuid-in-c-without-using-boost-library
                The implementation is not truly a 128-bit integer randomness
                but using a 64-bit random device right now. Clash rate should be extremely low even then
                for a scope that isnt incredibly huge.

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <iostream>
#include <xhash>    // made hashing available

namespace phy_uuid
{
    class UUID final
    {
    public:
        using value_type = std::uint64_t;
        static constexpr value_type Invalid = std::numeric_limits<value_type>::max();

        UUID();
        UUID(UUID const& other) = default;
        UUID(UUID&& other) : m_uuid(other.m_uuid) {
            
        }
        UUID& operator=(const UUID& other) = default;
        UUID& operator=(UUID&& other) = default;


        // Conversion constructor
        UUID(value_type uuid) : m_uuid{ uuid } {};

        // implicit converseions to value type
        operator value_type() { return m_uuid; }
        operator const value_type() const { return m_uuid; }

        value_type GetUUID() const { return m_uuid; }

    private:
        value_type m_uuid = Invalid;
    };
}

// hashing overload for UUID with std lib
namespace std
{
    template<>
    struct hash<phy_uuid::UUID>
    {
        size_t operator() (phy_uuid::UUID const& uuid) const
        {
            return hash<uint64_t>()((uint64_t)uuid);
        }
    };
}
