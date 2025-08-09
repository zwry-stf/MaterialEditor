#pragma once
#include <string>
#include <stdint.h>


typedef uint64_t fnv1a_hash_t;
namespace FNV1A
{
	inline constexpr uint64_t ullBasis = 0xCBF29CE484222325ULL;
	inline constexpr uint64_t ullPrime = 0x100000001B3ULL;

	consteval fnv1a_hash_t HashConst(const char* szString, const uint64_t uKey = ullBasis)
	{
		return (szString[0] == '\0') ? uKey : HashConst(&szString[1], (uKey ^ static_cast<uint64_t>(szString[0])) * ullPrime);
	}

	inline fnv1a_hash_t Hash(const char* szString, uint64_t uKey = ullBasis)
	{
		const size_t nLength = strlen(szString);

		for (size_t i = 0U; i < nLength; ++i)
		{
			uKey ^= szString[i];
			uKey *= ullPrime;
		}

		return uKey;
	}
}