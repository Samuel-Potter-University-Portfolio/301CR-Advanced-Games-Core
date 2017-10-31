#pragma once
#include "Common.h"


/**
* Generic factory for creating objects with an empty constructor
*/
template<class Type>
class CORE_API ClassFactory
{
private:
	/// Create 1 element per factory to get default set variables
	Type* m_control;
	uint32 m_id;

public:
	ClassFactory() 
	{
		m_control = New();
	}
	~ClassFactory() 
	{
		delete m_control;
	}

	/**
	* Create a new object of this factory's description
	* @returns New object (Allocated on heap)
	*/
	virtual Type* New() 
	{
		return new Type();
	}

	/**
	* Create a new object of this factory's description
	* @returns New object (Allocated on heap)
	*/
	template<class Out>
	Out* New()
	{
		return (Out*)New();
	}

	/**
	* Fetch the control object that was created when this factory was created
	* @returns control object (Should not be deleted)
	*/
	inline Type* GetControl() { return m_base; }

	/**
	* Fetch the name of the type that this factory was made using
	* @returns The name of this factory
	*/
	virtual const char* GetName() const
	{
		// Called 'class xyz' so remove class part
		return (typeid(Type).name() + 6);
	}

	/**
	* Fetch the hash for the type this factory produces
	* @returns 32 bit type hash (From compiler)
	*/
	virtual const uint32 GetHash() const 
	{
		return typeid(Type).hash_code();
	}


	inline void SetID(uint32 id) { m_id = id; }
	inline const uint32& GetID() const { return m_id; }
};


/**
* Create a generic factory for a given class
*/
template<class Parent, class Child>
class CORE_API ChildFactory : public ClassFactory<Parent>
{
public:
	/**
	* Create a new object of this factory's description
	* @returns New object (Allocated on heap)
	*/
	virtual Parent* New()
	{
		return new Child();
	}

	/**
	* Fetch the name of the type that this factory was made using
	* @returns The name of this factory
	*/
	virtual const char* GetName() const
	{
		// Called 'class xyz' so remove class part
		return (typeid(Child).name() + 6);
	}
	
	/**
	* Fetch the hash for the type this factory produces
	* @returns 32 bit type hash (From compiler)
	*/
	virtual const uint32 GetHash() const
	{
		return typeid(Child).hash_code();
	}
}; 