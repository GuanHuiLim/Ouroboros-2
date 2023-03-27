#pragma once
#include "rres.h"

#include <fstream>
#include <stdexcept>

namespace RRES
{
    namespace FILE_TYPES
    {
		static inline const unsigned char* NULLDATA = []() { return reinterpret_cast<const unsigned char*>("NULL"); }();
		static inline const unsigned char* RAWDATA  = []() { return reinterpret_cast<const unsigned char*>("RAWD"); }();
        static inline const unsigned char* TEXT     = []() { return reinterpret_cast<const unsigned char*>("TEXT"); }();
        static inline const unsigned char* IMGE     = []() { return reinterpret_cast<const unsigned char*>("IMGE"); }();
        static inline const unsigned char* WAVE     = []() { return reinterpret_cast<const unsigned char*>("WAVE"); }();
        static inline const unsigned char* VRTX     = []() { return reinterpret_cast<const unsigned char*>("VRTX"); }();
        static inline const unsigned char* FNTG     = []() { return reinterpret_cast<const unsigned char*>("FNTG"); }();
        static inline const unsigned char* LINK     = []() { return reinterpret_cast<const unsigned char*>("LINK"); }();
        static inline const unsigned char* CDIR     = []() { return reinterpret_cast<const unsigned char*>("CDIR"); }();
    }; //namespace FILE_TYPES

    static constexpr inline const unsigned int CHUNK_DATA_PROPS_SIZE = 5 * sizeof(unsigned int);
    struct Asset
    {
        rresResourceChunkInfo m_info = []() {
            decltype(m_info) info;
            memcpy(info.type, FILE_TYPES::RAWDATA, sizeof(info.type));
			info.id = 0;
            info.compType = rresCompressionType::RRES_COMP_NONE;
            info.cipherType = rresEncryptionType::RRES_CIPHER_NONE;
            info.flags = 0;
            info.baseSize = 0;
            info.packedSize = 0;
            info.nextOffset = 0;
            info.reserved = 0;
            info.crc32 = 0;
			return info;
        }();

		std::array<unsigned int, 1> m_props = []() {
			decltype(m_props) props;
			props[0] = 0;
			return props;
		}();

        rresResourceChunkData m_chunkData = [&]() {
            decltype(m_chunkData) chunkData;
            chunkData.propCount = 1;
            chunkData.props = m_props.data();
            return chunkData;
        }();

        std::unique_ptr<unsigned char[]> m_data{};

        inline unsigned int CalculateChunkID(const char* filename) const noexcept
        {
            std::string filename_str(filename);
            return rresComputeCRC32(reinterpret_cast<unsigned char*>(filename_str.data()), filename_str.length());;
        }

    public:
        Asset(const char* filename, const unsigned char* type = FILE_TYPES::RAWDATA,
            rresCompressionType compType = rresCompressionType::RRES_COMP_NONE,
            rresEncryptionType cipherType = rresEncryptionType::RRES_CIPHER_NONE, 
            unsigned int flags = 0);
    };
    
    class AssetBundle
    {
    private:
        std::string m_filename{};
        rresFileHeader m_header = {
            { 'r', 'r', 'e', 's' }, // File identifier: rres
            100,                    // File version: 100 for version 1.0
            0,                      // Number of resource chunks in the file (MAX: 65535)
            0,                      // Central Directory offset in file (0 if not available)
            0                       // <reserved>
        };

        std::vector<Asset> m_assets{};
    public:
        AssetBundle(const char* filename);

        ~AssetBundle();

        void AddAsset(const char* filename, const unsigned char* type = FILE_TYPES::RAWDATA, rresCompressionType compType = rresCompressionType::RRES_COMP_NONE,
            rresEncryptionType cipherType = rresEncryptionType::RRES_CIPHER_NONE, unsigned int flags = 0);

		void WriteToFile();
        void WriteToFile(const char* filename);
    };
}; //namespace RRES


