#pragma once
#include <filesystem>

namespace tik
{
    namespace hamming
    {
        void encode(const std::filesystem::path& from,
                    const std::filesystem::path& to);

        void encode_not_extended(const std::filesystem::path& from,
                                 const std::filesystem::path& to);

        void decode(const std::filesystem::path& from,
                    const std::filesystem::path& to);
    }
}
