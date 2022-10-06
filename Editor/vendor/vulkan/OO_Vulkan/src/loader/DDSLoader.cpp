/************************************************************************************//*!
\file           DDSLoader.cpp
\project        Ouroboros
\author         Jamie Kong, j.kong, 390004720 | code contribution (100%)
\par            email: j.kong\@digipen.edu
\date           Oct 02, 2022
\brief              Defines DDS loader to help with loading files

Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*//*************************************************************************************/
#include "DDSLoader.h"

#include "tinyddsloader.h"
#include "VulkanUtils.h"

namespace oGFX
{
    void LoadDDS(const std::string& filename, oGFX::FileImageData& data)
    {
        using namespace tinyddsloader;

        DDSFile dds;
        auto ret = dds.Load(filename.c_str());
        if (tinyddsloader::Result::Success != ret)
        {
            return;
        }

        auto getFormat = [&]()->VkFormat
        {
            switch (dds.GetFormat())
            {
            case DDSFile::DXGIFormat::R8G8B8A8_UNorm:
            case DDSFile::DXGIFormat::R8G8B8A8_UNorm_SRGB:
            return (data.imgType == FileImageData::ImageType::LINEAR) ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_SRGB;
            break;

            case DDSFile::DXGIFormat::B8G8R8A8_UNorm:
            case DDSFile::DXGIFormat::B8G8R8A8_UNorm_SRGB:
            return  (data.imgType == FileImageData::ImageType::LINEAR) ? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_B8G8R8A8_SRGB;
            break;


            case DDSFile::DXGIFormat::B8G8R8X8_UNorm:
            case DDSFile::DXGIFormat::B8G8R8X8_UNorm_SRGB:
            break;

            case DDSFile::DXGIFormat::BC1_UNorm:
            case DDSFile::DXGIFormat::BC1_UNorm_SRGB:
            return (data.imgType == FileImageData::ImageType::LINEAR) ? VK_FORMAT_BC1_RGBA_UNORM_BLOCK : VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
            break;

            case DDSFile::DXGIFormat::BC2_UNorm:
            case DDSFile::DXGIFormat::BC2_UNorm_SRGB:
            return  (data.imgType == FileImageData::ImageType::LINEAR) ? VK_FORMAT_BC2_UNORM_BLOCK : VK_FORMAT_BC2_SRGB_BLOCK;
            break;

            case DDSFile::DXGIFormat::BC3_UNorm:
            case DDSFile::DXGIFormat::BC3_UNorm_SRGB:
            return  (data.imgType == FileImageData::ImageType::LINEAR) ? VK_FORMAT_BC3_UNORM_BLOCK : VK_FORMAT_BC3_SRGB_BLOCK;
            break;

            case DDSFile::DXGIFormat::BC5_UNorm:
            return  (data.imgType == FileImageData::ImageType::LINEAR) ? VK_FORMAT_BC5_UNORM_BLOCK : VK_FORMAT_UNDEFINED;

            break;

            default:
            return VK_FORMAT_B8G8R8A8_SRGB;
            break;
            }
            return VK_FORMAT_B8G8R8A8_SRGB;
        };

        data.format = getFormat();

        uint64_t dataSize{};
        for (size_t i = 0; i < dds.GetMipCount(); i++)
        {
            const auto imageInformation = dds.GetImageData( static_cast<uint32_t>(i), 0);

            VkBufferImageCopy copyRegion{};
            copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.imageSubresource.mipLevel = static_cast<uint32_t>(i);
            copyRegion.imageSubresource.baseArrayLayer = 0;
            copyRegion.imageSubresource.layerCount = 1;
            copyRegion.bufferOffset = dataSize;
            copyRegion.imageExtent.width = imageInformation->m_width;
            copyRegion.imageExtent.height = imageInformation->m_height;
            copyRegion.imageExtent.depth = 1;

            data.mipInformation.push_back(copyRegion);
            dataSize += imageInformation->m_memSlicePitch;
        }

        data.imgData.resize(dataSize);
        for (size_t i = 0; i <  dds.GetMipCount(); i++)
        {
            const auto imageInformation = dds.GetImageData( static_cast<uint32_t>(i) , 0);
            memcpy(data.imgData.data() +data.mipInformation[i].bufferOffset  ,imageInformation->m_mem , imageInformation->m_memSlicePitch);
        }
        

        const auto imageInformation = dds.GetImageData(0, 0);
        data.w = imageInformation->m_width;
        data.h =  imageInformation->m_height;
        data.dataSize = dataSize;        

    }
}
