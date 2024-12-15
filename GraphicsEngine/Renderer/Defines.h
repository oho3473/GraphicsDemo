#pragma once
#include <stdexcept>

#ifndef HR_CHECK
// ��ũ�ο��� do while ���� ����ϸ� ����. �����ݷ� ����.
#define HR_CHECK(hr)												\
	do																\
{																	\
		HRESULT hr_result = hr;										\
		if (FAILED(hr_result))										\
		{															\
			const char* msg = nullptr;								\
			[&]() -> const char*									\
			{														\
				static char s_str[64] = {};							\
				sprintf_s(s_str, "Failure with HRESULT of %08X",	\
					static_cast<unsigned int>(hr_result));			\
				msg = s_str;										\
				return msg;											\
			}();													\
			throw std::runtime_error(msg);							\
		}															\
	} while(0)
#endif

#define SAFE_RELEASE_COM(x)											\
	do																\
	{																\
		if ((x) != nullptr)											\
		{															\
			(x)->Release();											\
			(x) = nullptr;											\
		}															\
	} while (0)

