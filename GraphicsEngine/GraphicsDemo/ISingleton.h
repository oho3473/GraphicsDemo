#pragma once
template<typename T>
class ISingleton
{
public:
	virtual ~ISingleton() {}

	static T* GetInstance();
	static void ReleaseInstance();

protected:
	ISingleton() {}

	static T* returnSingleton;
};

template<typename T>
T* ISingleton<T>::returnSingleton = nullptr;

template<typename T>
T* ISingleton<T>::GetInstance()
{
	if (returnSingleton == nullptr)
	{
		returnSingleton = new T();
		return returnSingleton;
	}

	return returnSingleton;
}


template<typename T>
void ISingleton<T>::ReleaseInstance()
{
	if (returnSingleton)
	{
		delete returnSingleton;
		returnSingleton = nullptr;
	}
}

