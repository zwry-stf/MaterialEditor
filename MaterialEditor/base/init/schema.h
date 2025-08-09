#pragma once
#include <stdint.h>
#include <string>
#include <type_traits>


class schema
{
public:
	static uint32_t getOffset(const std::string& path);
};

template<typename T>
using SmartReturn = std::conditional_t<
	(sizeof(T) <= 2 * sizeof(void*) && std::is_trivially_copy_constructible_v<T>) ||
	std::is_trivially_move_constructible_v<T>,
	T,
	const T&
>;

#define Add_Offset(_class, _name, _type) \
__forceinline _type& _name() { \
    static int offset = schema::getOffset(#_class "->" #_name); \
\
	return *reinterpret_cast<_type*>((unsigned __int64)this + offset); \
} \
\
__forceinline SmartReturn<_type> _name() const { \
    static int offset = schema::getOffset(#_class "->" #_name); \
\
	return *reinterpret_cast<_type*>((unsigned __int64)this + offset); \
}

// Adds an offset to a pointer without dereferencing
#define Add_Offset_Ptr(_class, _name, _type) \
__forceinline _type* _name() const { \
    static int offset = schema::getOffset(#_class "->" #_name); \
\
	return reinterpret_cast<_type*>((unsigned __int64)this + offset); \
}

// Static offsets
#define Add_Offset_Static(_name, _type, _offset) \
__forceinline _type& _name() { \
	return *reinterpret_cast<_type*>((unsigned __int64)this + _offset); \
} \
\
__forceinline SmartReturn<_type> _name() const { \
	return *reinterpret_cast<_type*>((unsigned __int64)this + _offset); \
}

#define Add_Offset_Static_Ptr(_name, _type, _offset) \
__forceinline _type* _name() const { \
	return reinterpret_cast<_type*>((unsigned __int64)this + _offset); \
}