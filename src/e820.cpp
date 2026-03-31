#include "vsos/memory/e820.hpp"

namespace vsos::memory::e820
{
    namespace
    {
        constexpr size_t MAX_ENTRIES = 128;

        Entry s_entries[MAX_ENTRIES];
        size_t s_entry_count = 0;

        size_t s_usable_indices[MAX_ENTRIES];
        size_t s_usable_count = 0;

        MapView s_map{ s_entries, 0 };

        constexpr MemoryType convert_type(uint32_t raw) noexcept
        {
            switch (raw)
            {
            case 1u: return MemoryType::Usable;
            case 2u: return MemoryType::Reserved;
            case 3u: return MemoryType::ACPIReclaim;
            case 4u: return MemoryType::ACPINVS;
            case 5u: return MemoryType::BadMemory;
            default: return MemoryType::Unknown;
            }
        }

        inline uint64_t end_of(const Entry& e) noexcept
        {
            return e.base + e.length;
        }

        void sort_by_base(Entry* entries, size_t count) noexcept
        {
            // 単純挿入ソート（件数小さい前提・動的確保禁止）
            for (size_t i = 1; i < count; ++i)
            {
                Entry key = entries[i];
                size_t j = i;
                while (j > 0 && entries[j - 1].base > key.base)
                {
                    entries[j] = entries[j - 1];
                    --j;
                }
                entries[j] = key;
            }
        }

        size_t normalize_and_merge(Entry* entries, size_t count) noexcept
        {
            if (count == 0)
                return 0;

            sort_by_base(entries, count);

            size_t write = 0;
            Entry current = entries[0];

            if (current.length == 0)
            {
                // 0 長はスキップ
                size_t i = 1;
                for (; i < count; ++i)
                {
                    if (entries[i].length != 0)
                    {
                        current = entries[i];
                        break;
                    }
                }
                if (current.length == 0)
                    return 0;
            }

            for (size_t i = 1; i < count; ++i)
            {
                const Entry& e = entries[i];
                if (e.length == 0)
                    continue;

                // 同じ type かつ連続 or 重なり → マージ
                if (e.type == current.type && e.base <= end_of(current))
                {
                    const uint64_t new_end = (end_of(e) > end_of(current)) ? end_of(e) : end_of(current);
                    current.length = new_end - current.base;
                }
                else
                {
                    entries[write] = current;
                    ++write;
                    current = e;
                }
            }

            entries[write] = current;
            ++write;

            return write;
        }

        void rebuild_usable_indices() noexcept
        {
            s_usable_count = 0;
            for (size_t i = 0; i < s_entry_count && s_usable_count < MAX_ENTRIES; ++i)
            {
                if (s_entries[i].type == MemoryType::Usable)
                {
                    s_usable_indices[s_usable_count] = i;
                    ++s_usable_count;
                }
            }
        }
    } // namespace

    void init(const BootInfo& boot_info) noexcept
    {
        s_entry_count = 0;
        s_usable_count = 0;
        s_map.entries = s_entries;
        s_map.count = 0;

        if (boot_info.entry_count == 0 || boot_info.entries == nullptr)
            return;

        const uint32_t src_count = boot_info.entry_count;
        const RawEntry* src = boot_info.entries;

        for (uint32_t i = 0; i < src_count && s_entry_count < MAX_ENTRIES; ++i)
        {
            const RawEntry& r = src[i];
            if (r.length == 0)
                continue;

            Entry& e = s_entries[s_entry_count];
            e.base   = r.base;
            e.length = r.length;
            e.type   = convert_type(r.type);

            ++s_entry_count;
        }

        s_entry_count = normalize_and_merge(s_entries, s_entry_count);
        s_map.count   = s_entry_count;

        rebuild_usable_indices();
    }

    MapView get_map() noexcept
    {
        return s_map;
    }

    size_t get_usable_count() noexcept
    {
        return s_usable_count;
    }

    const Entry* get_usable_entry(size_t index) noexcept
    {
        if (index >= s_usable_count)
            return nullptr;

        const size_t real = s_usable_indices[index];
        if (real >= s_entry_count)
            return nullptr;

        return &s_entries[real];
    }
}
