#pragma once


/**
* Smart pointer that will store a variable as long as there is a reference to it
* https://www.codeproject.com/Articles/15351/Implementing-a-simple-smart-pointer-in-c
*/
template<typename Type>
class SafePtr 
{
public:
	typedef unsigned short RefCounter;

private:
	Type* data;
	RefCounter* refs;

public:
	SafePtr(Type* data = nullptr) : data(data == nullptr ? new Type : data), refs(new RefCounter(0))
	{
		++(*refs);
	}

	SafePtr(const SafePtr<Type>& other) : data(other.data), refs(other.refs)
	{
		++(*refs);
	}

	~SafePtr() 
	{
		if (--(*refs) == 0)
		{
			delete data;
			delete refs;
		}
	}

	inline Type& operator*() { return *data; }
	inline const Type& operator*() const { return *data; }

	inline Type* operator->() { return data; }
	inline const Type* operator->() const { return data; }

	inline SafePtr<Type>& operator=(const SafePtr<Type>& other)
	{
		if (this != &other)
		{
			// Cleanup previous ptr
			if (--(*refs) == 0)
			{
				delete data;
				delete refs;
			}

			data = other.data;
			refs = other.refs;
			++(*refs);
		}
		return *this;
	}
};