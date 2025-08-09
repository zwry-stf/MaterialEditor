#pragma once
#include <string>
#include <vector>
#include <array>
#include <assert.h>


struct SignatureData
{
	int16_t* pData;
	size_t nLength;
	uint8_t nKey;
	int nPreOffset;
};

template <size_t N>
class SignatureString
{
private:
	uint8_t key_ = 0;
	size_t length_ = 0;
	int preOffset_ = 0;
	int16_t arr_[N + 4] = { 0 };

public:
	consteval SignatureString(const char(&text)[N])
	{
		static_assert(N > 1, "signature to small");
		key_ = generate_key(static_cast<uint32_t>(text[0]));

		char currBuffer[2];
		int currBufferLength = 0;

		size_t len = N; // remove 0
		for (size_t i = 0; i < len; i++)
		{
			if (text[i] == ' ' || text[i] == '\0')
			{
				if (currBufferLength == 0)
					continue;

				if (currBufferLength > 2)
					throw "byte to big";

				if (currBufferLength == 1)
				{
					if (currBuffer[0] == '?')
						add(-1);
					else
						add(static_cast<int16_t>(hex_value(currBuffer[0])));
				}
				else if (currBufferLength == 2)
				{
					if (currBuffer[0] == '?' &&
						currBuffer[1] == '?')
						add(-1);
					else if (currBuffer[0] == '{' &&
						currBuffer[1] == '}')
					{
						preOffset_ = static_cast<int>(length_);
						add(-1);
						add(-1);
						add(-1);
						add(-1);
					}
					else
						add(static_cast<int16_t>(parse_hex(currBuffer[0], currBuffer[1])));
				}

				currBufferLength = 0;
				continue;
			}

			if (currBufferLength > 1)
				throw "byte to big";

			currBuffer[currBufferLength] = text[i];
			currBufferLength++;
		}
	}

public:
	SignatureData getData() const
	{
		SignatureData data;
		data.nKey = key_;
		data.nLength = length_;
		data.pData = (int16_t*)&arr_;
		data.nPreOffset = preOffset_;

		return data;
	}

private:
	constexpr size_t add(int16_t c)
	{
		arr_[length_] = c ^ key_;
		length_++;
		return length_;
	}
	constexpr uint8_t hex_value(char c)
	{
		if (c >= '0' && c <= '9') return c - '0';
		if (c >= 'A' && c <= 'F') return c - 'A' + 10;
		if (c >= 'a' && c <= 'f') return c - 'a' + 10;

		throw "invalid hex digit";
	}

	constexpr uint8_t parse_hex(char hi, char lo)
	{
		return hex_value(hi) * 16 + hex_value(lo);
	}

	constexpr static uint8_t generate_key(uint32_t counter)
	{
		uint8_t ret = 0;
		while (ret == 0)
		{
			constexpr char const* t = __TIME__;
			uint8_t time_key = static_cast<uint8_t>((t[7] - '0') * 60 + (t[6] - '0') * 60 * 10
				+ (t[4] - '0') * 10 + (t[3] - '0')
				+ (t[1] - '0') * 3600 + (t[0] - '0') * 3600 * 10);
			ret = static_cast<uint8_t>(time_key ^ (counter & 0xFF));
			counter++;
		}

		return ret;
	}
};

class signatures
{
public:
	static void init();

	static void* patternScan(void* start, void* end, const std::vector<int16_t>& pattern, int pre_offset, int post_offset);
	static void* patternScan(const std::string& module, const std::vector<int16_t>& pattern, int pre_offset, int post_offset);

	template <typename T>
	inline static T get(const std::string& name)
	{
		T ret;
		getSigEx(name, (void**)&ret);
		return ret;
	}

	inline static int32_t getOffset(const std::string& name)
	{
		void* ret;
		getSigEx(name, (void**)&ret);

		int32_t ret_;
		memcpy(&ret_, &ret, sizeof(ret_));

		return ret_;
	}

private:
	template <typename T, size_t N>
	inline static void addSig(const std::string& name, const std::string& module, const SignatureString<N>& sig, T& out, int post_offset = 0)
	{
		addSigEx(name, module, sig.getData(), (void**)&out, post_offset);
	}

	template <size_t N>
	inline static void addSig(const std::string& name, const std::string& module, const SignatureString<N>& sig)
	{
		addSigEx(name, module, sig.getData(), nullptr, 0);
	}

	template <size_t N>
	inline static void addSigOffset(const std::string& name, const std::string& module, const SignatureString<N>& sig)
	{
		addSigOffsetEx(name, module, sig.getData());
	}

	static void addSigEx(const std::string& name, const std::string& module, const SignatureData& sig, void** out, int post_offset);
	static void addSigOffsetEx(const std::string& name, const std::string& module, const SignatureData& sig);

	static void getSigEx(const std::string& name, void** out);

private:
	static void addSignatures();

private:
	inline static bool failed = false;
};