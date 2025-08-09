#pragma once
#include <util/mem.h>
#include <stdint.h>
#include <init/functions.h>
#include <init/signatures.h>


enum StringConvertErrorPolicy {
    STRING_CONVERT_FLAG_SKIP = 1,
    STRING_CONVERT_FLAG_FAIL = 2,
    STRING_CONVERT_FLAG_ASSERT = 4,

    STRING_CONVERT_REPLACE = 0,
    STRING_CONVERT_SKIP = STRING_CONVERT_FLAG_SKIP,
    STRING_CONVERT_FAIL = STRING_CONVERT_FLAG_FAIL,

    STRING_CONVERT_ASSERT_REPLACE = STRING_CONVERT_FLAG_ASSERT + STRING_CONVERT_REPLACE,
    STRING_CONVERT_ASSERT_SKIP = STRING_CONVERT_FLAG_ASSERT + STRING_CONVERT_SKIP,
    STRING_CONVERT_ASSERT_FAIL = STRING_CONVERT_FLAG_ASSERT + STRING_CONVERT_FAIL,
};

template <size_t N>
consteval uint64_t BuildExtension(const char(&ext)[N])
{
    static_assert(N == 5, "Invalid extension");

    return ext[3] << 24 | ext[2] << 16 | ext[1] << 8 | ext[0];
}

class CBufferString
{
public:
    // enums
    enum AllocationOption_t {
        UNK1 = -1,
        UNK2 = 0,
        UNK3 = (1 << 1),
        UNK4 = (1 << 8),
        UNK5 = (1 << 9)
    };

    enum AllocationFlags_t {
        iLengthMASK = (1 << 30) - 1,
        FLAGS_MASK = ~iLengthMASK,

        OVERFLOWED_MARKER = (1 << 30),
        FREE_HEAP_MARKER = (1 << 31),

        STACK_ALLOCATED_MARKER = (1 << 30),
        ALLOW_HEAP_ALLOCATION = (1 << 31)
    };

    // constructors
    inline CBufferString(bool allow_heap_allocation = true) :
        iLength(0),
        iAllocatedSize((allow_heap_allocation* ALLOW_HEAP_ALLOCATION) | STACK_ALLOCATED_MARKER | sizeof(cString)),
        cStringPtr(nullptr)
    {
    }

    inline CBufferString(const char* string, bool allow_heap_allocation = true) :
        CBufferString(allow_heap_allocation)
    {
        insert(0, string);
    }

    inline CBufferString(const char* string, uint64_t extension)
        : CBufferString(true)
    {
        fixup(string, extension);
    }

    void fixup(const char* string, uint64_t extension)
    {
        static auto oFixupBufferString = signatures::get<
            bool(__fastcall*)(CBufferString*, const char*, uint64_t)>
            ("FixupResourceClient");

        oFixupBufferString(this, string, extension);

        this->m_nUnknown4 = extension;
    }

    inline CBufferString(const CBufferString& other) : CBufferString() { *this = other; }

    // deconstructor
    inline ~CBufferString() { purge(); }

    // functions
    void SetHeapAllocationState(bool bState) {
        if (bState)
            iAllocatedSize |= ALLOW_HEAP_ALLOCATION;
        else
            iAllocatedSize &= ~ALLOW_HEAP_ALLOCATION;
    }

    int allocated_num() const { return iAllocatedSize & iLengthMASK; }
    int length() const { return iLength & iLengthMASK; }

    bool can_heap_allocate() const { return (iAllocatedSize & ALLOW_HEAP_ALLOCATION) != 0; }
    bool is_stack_allocated() const { return (iAllocatedSize & STACK_ALLOCATED_MARKER) != 0; }
    bool should_free_memory() const { return (iLength & FREE_HEAP_MARKER) != 0; }
    bool is_overflowed() const { return (iLength & OVERFLOWED_MARKER) != 0; }

    bool is_input_string_unsafe(const char* data) const {
        return ((void*)data >= this && (void*)data < &this[1]) ||
            (!is_allocation_empty() && data >= base() && data < (base() + allocated_num()));
    }

    bool is_allocation_empty() const { return allocated_num() == 0; }

    void set_extension(const char* ext, int unk = 0)
    {
        static auto oBufferStringSetExtension = functions::get<
            const char* (__fastcall*)(CBufferString*, const char*, int)>
            ("CBufferString::SetExtension");

        oBufferStringSetExtension(this, ext, unk);
    }

public:
    char* base() { return is_stack_allocated() ? cString : (!is_allocation_empty() ? cStringPtr : nullptr); }
    const char* base() const { return const_cast<CBufferString*>(this)->base(); }

public:
    void clear() {
        if (!is_allocation_empty())
            base()[0] = '\0';

        iLength &= ~iLengthMASK;
    }

public:
    const char* insert(int index, const char* buf, int count = -1, bool ignore_alignment = false)
    {
        static auto oBufferStringInsert = functions::get<
            const char* (__fastcall*)(CBufferString*, int, const char*, int, bool)>
            ("CBufferString::Insert");

        return oBufferStringInsert(this, index, buf, count, ignore_alignment);
    }

    void purge(int allocated_bytes_to_preserve = 0)
    {
        static auto oBufferStringPurge = functions::get<
            void(__fastcall*)(CBufferString*, int)>
            ("CBufferString::Purge");

        oBufferStringPurge(this, allocated_bytes_to_preserve);
    }

public:
    int iLength;
    int iAllocatedSize;

    union {
        char* cStringPtr;
        char cString[0xC8];
    };

    uint64_t m_nUnknown3 = 0;
    uint64_t m_nUnknown4 = 0;
};