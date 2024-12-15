#pragma once

#include <string>

class Util
{
public:
	template <typename T>
	static std::wstring ToWideChar(const T& value)
	{
		std::wstringstream wss;
		wss << value;
		return wss.str();
	}

	template <>
	static std::wstring ToWideChar(const std::string& str)
	{
		int num_chars = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), nullptr, 0);
		std::wstring wstrTo;
		if (num_chars)
		{
			wstrTo.resize(num_chars);
			MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &wstrTo[0], num_chars);
		}
		return wstrTo;
	}

	static std::string ToMultiByte(const std::wstring& wstr)
	{
		int num_chars = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), nullptr, 0, nullptr, nullptr);
		std::string strTo;
		if (num_chars > 0)
		{
			strTo.resize(num_chars);
			WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), &strTo[0], num_chars, nullptr, nullptr);
		}
		return strTo;
	}

	// 16바이트 배수로 크기 정렬
	constexpr UINT AlignTo16(UINT size)
	{
		return (size + 15) & ~15;
	}

};

