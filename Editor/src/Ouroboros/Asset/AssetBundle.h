#include "rres.h"

#include <cstdio>
#include <cstring>
#include <stdexcept>

class rresFile
{
public:
    rresFile(const char* filename)
        : m_file(fopen(filename, "wb"))
    {
        if (!m_file)
            throw std::runtime_error("Failed to open file");
        fwrite(&m_header, sizeof(m_header), 1, m_file);
    }

    ~rresFile()
    {
        if (m_file)
            fclose(m_file);
    }

    void addResourceChunk(const char* filename, const char* type, rresCompressionType compType = rresCompressionType::RRES_COMP_NONE,
        rresEncryptionType cipherType = rresEncryptionType::RRES_CIPHER_NONE, unsigned int flags = 0)
    {
        unsigned char* buffer = nullptr;
        unsigned int rawSize = 0;

        // Load file data
        FILE* file = fopen(filename, "rb");
        if (file)
        {
            fseek(file, 0, SEEK_END);
            rawSize = static_cast<unsigned int>(ftell(file));
            fseek(file, 0, SEEK_SET);
            buffer = new unsigned char[rawSize];
            fread(buffer, 1, rawSize, file);
            fclose(file);
        }
        else
        {
            throw std::runtime_error("Failed to open file");
        }

        // Define chunk info
        rresResourceChunkInfo chunkInfo = { 0 };
        memcpy(chunkInfo.type, type, sizeof(chunkInfo.type));
        chunkInfo.id = rresComputeCRC32(filename, strlen(filename));
        chunkInfo.compType = compType;
        chunkInfo.cipherType = cipherType;
        chunkInfo.flags = flags;
        chunkInfo.baseSize = 5 * sizeof(unsigned int) + rawSize;
        chunkInfo.packedSize = chunkInfo.baseSize;
        chunkInfo.nextOffset = 0;
        chunkInfo.reserved = 0;

        // Define chunk data
        rresResourceChunkData chunkData = { 0 };
        chunkData.propCount = 1;
        chunkData.props = new unsigned int[chunkData.propCount];
        chunkData.props[0] = rawSize;

        fwrite(&chunkInfo, sizeof(rresResourceChunkInfo), 1, m_file);
        fwrite(chunkData.props, sizeof(unsigned int), chunkData.propCount, m_file);
        fwrite(buffer, 1, rawSize, m_file);

        // Free resources
        delete[] chunkData.props;
        delete[] buffer;

        // Increment chunk count in header
        ++m_header.chunkCount;
    }

private:
    FILE* m_file = nullptr;
    rresFileHeader m_header = {
        { 'r', 'r', 'e', 's' },
        100,
        0,
        0,
        0
    };
};
