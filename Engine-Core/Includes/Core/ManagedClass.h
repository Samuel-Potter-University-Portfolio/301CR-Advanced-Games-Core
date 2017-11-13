#pragma once
#include "Common.h"



/**
* Store start id (Used to get compile time unique ids)
* 0-9,999		Reserved for engine (lib)
* 10,000-xyz	Reserved for game application (exe)
*/
#ifdef BUILD_CORE
#define __ID_START__ 0
#else
#define __ID_START__ 10000
#endif
#define __UNIQUE_ID__ (__ID_START__ + __COUNTER__)



/**
* Managed class, an easily accessible class factory 
* and unique identifier for a class
* -Note: This should never be made outside of the managed macros
*/
class CORE_API MClass
{
private:
	const char* m_name;
	const uint16 m_id;

public:
	MClass(const char* name, const uint16& id);

	/**
	* Generates a new object of this class type
	* @param dst		The destination to make this object at (Leave as nullptr, if you want default allocation)
	* @returns New object allocated at dst or on heap
	*/
	virtual class ManagedObject* NewObject(void* dst = nullptr) const;
	/**
	* Generates a new object of this class type and casts it to desired type
	* @param dst		The destination to make this object at (Leave as nullptr, if you want default allocation)
	* @returns New object allocated at dst or on heap
	*/
	template<class Type>
	inline Type* New(void* dst = nullptr) const 
	{
		return static_cast<Type*>(NewObject(dst));
	}

	/**
	* Get the parent class of this class
	* @returns The parent class, if exists or nullptr
	*/
	virtual const MClass* GetParentClass() const;

	/**
	* Is this class a child of the the other class (Returns true if the same class)
	* @param other				The class we want to check if they are our parent
	* @returns True, if the other class is us or a parent
	*/
	bool IsChildOf(const MClass* other) const;


	/**
	* Getters & Setters
	*/
public:
	inline const char* GetName() const { return m_name; }
	inline const uint16& GetID() const { return m_id; }
};


/**
* Ensures that only classes which extend this template type can be stored here
* -If attempt to set to a class which doesn't, it will just not change
*/
template<class Type>
class SubClassOf 
{
private:
	const MClass* m_type = Type::StaticClass();
public:
	inline SubClassOf<Type>& operator=(const MClass* value)
	{
		if (value->IsChildOf(Type::StaticClass()))
			m_type = value;
		return *this;
	}
	template<class OtherType>
	inline SubClassOf<Type>& operator=(const SubClassOf<OtherType>& value) { return operator=(value.m_type); }


	template<class OtherType>
	inline bool operator==(const SubClassOf<OtherType>& other) const { return m_type == other.m_type; }
	template<class OtherType>
	inline bool operator!=(const SubClassOf<OtherType>& other) const { return m_type != other.m_type; }

	inline bool operator==(const MClass* other) const { return m_type == other; }
	inline bool operator!=(const MClass* other) const { return m_type != other; }

	inline const MClass* operator*() const { return m_type; }
	inline const MClass* operator->() const { return m_type; }
};


/**
* A child of this class, will have a managed class automatically generated for it through the following macros:
* -CLASS_BODY() as the first line in the class decleration
* -CLASS_SOURCE(<Class Name>, <Parent Class Name>, <API>) in the translation file (cpp)
* This object should also implement a default parameterless constructor that will be called by the class during construction
* -NOTE: Never inherit from multiple (children of) ManagedObject classes
*/
class CORE_API ManagedObject
{
public:
	/** Return single class instance used by this class type */
	inline static const MClass* StaticClass();

	/** Return single class instance used by this class	*/
	virtual const MClass* GetClass() = 0;
};




/**
* Generates required function signatures for managed objects
*/
#define CLASS_BODY() \
public: \
	static const MClass* StaticClass(); \
	virtual const MClass* GetClass(); \
private:


/**
* Generates the required MClass and function content for Managed objects
*/
#define CLASS_SOURCE(ClassName, ParentName, API) \
class API ClassName ## _Class : public MClass \
{ \
private: \
	friend class ClassName; \
	ClassName m_refObj; \
	\
	ClassName ## _Class() : MClass(#ClassName, __UNIQUE_ID__) {} \
\
	virtual ManagedObject* NewObject(void* dst = nullptr) const { return dst == nullptr ? new ClassName : new(dst) ClassName; } \
\
	virtual const MClass* GetParentClass() const { return ParentName::StaticClass(); } \
public: \
	const ClassName* GetReferenceObject() const { return &m_refObj; } \
}; \
\
const MClass* ClassName::StaticClass() { static ClassName ## _Class sc; return &sc; } \
const MClass* ClassName::GetClass() { return ClassName::StaticClass(); } 