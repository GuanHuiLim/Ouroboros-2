/************************************************************************************//*!
\file          FileDropEvent.h
\project       Ouroboros
\author        Jamie Kong, j.kong , 390004720 | code contribution (100%)
\par           email: j.kong\@digipen.edu
\date          November 3, 2022
\brief         Implements an event related to dropping of files into the window.
                Used to extract info from drop events such as file directory and time

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#pragma once

#include <filesystem>
#include "Ouroboros/Core/Events/AppEvent.h"
#include <string>

namespace oo
{
    enum class FileDropType
    {
        DropBegin,
        DropFile,
        DropText,
        DropEnd,
    };

    /********************************************************************************//*!
     @brief     Implements a File Dropped AppEvent.
    *//*********************************************************************************/
    class FileDropEvent final : public AppEvent
    {
    public:
        FileDropEvent(FileDropType type, std::filesystem::path path, uint32_t window, uint32_t time)
            : m_type{ type }, m_file{ path },
            m_windowID{ window }, m_timestamp{ time }{}

        FileDropType GetType() const { return m_type; }
        std::filesystem::path GetFile() const { return m_file; }
        uint32_t GetWindowDropped() const { return m_windowID; }
        uint32_t GetTimeDropped()const { return m_timestamp; }

        std::string ToString() const override final
        {
            std::stringstream ss;
            ss << "FileDropEvent : " << m_file;
            return ss.str();
        }

        EVENT_CLASS_TYPE(FILEDROPPED);
        EVENT_CLASS_CATEGORY(bitmask{ EVENT_CATEGORY::FILEDROP } | EVENT_CATEGORY::INPUT) //TECHNICALLY ITS INPUT
    private:
        FileDropType m_type;
        std::filesystem::path m_file;
        uint32_t m_windowID;
        uint32_t m_timestamp;
    };

}// End namespace oo

