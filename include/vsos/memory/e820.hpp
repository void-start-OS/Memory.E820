#pragma once

#include <stdint.h>
#include <stddef.h>

namespace vsos::memory::e820
{
    // BIOS E820 type
    enum class MemoryType : uint32_t
    {
        Usable        = 1,
        Reserved      = 2,
        ACPIReclaim   = 3,
        ACPINVS       = 4,
        BadMemory     = 5,
        Unknown       = 0xFFFFFFFFu
    };

    struct Entry
    {
        uint64_t base;
        uint64_t length;
        MemoryType type;
    };

    struct MapView
    {
        const Entry* entries;
        size_t count;
    };

    // Core から渡される生データ（ABI 固定用）
    struct RawEntry
    {
        uint64_t base;
        uint64_t length;
        uint32_t type;
        uint32_t acpi_ext;
    };

    struct BootInfo
    {
        uint32_t entry_count;
        const RawEntry* entries;
    };

    // 初期化：BootInfo から内部マップを構築（ソート・マージ・フィルタ含む）
    void init(const BootInfo& boot_info) noexcept;

    // 全エントリ（正規化済み）のビュー
    MapView get_map() noexcept;

    // Usable のみの件数
    size_t get_usable_count() noexcept;

    // i 番目の Usable エントリ（範囲外なら nullptr）
    const Entry* get_usable_entry(size_t index) noexcept;
}
