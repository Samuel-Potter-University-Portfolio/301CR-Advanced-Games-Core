#pragma once
#include "Common.h"



/**
* Store start id (Used to get compile time unique ids)
* 0-29,999		Reserved for engine (lib)
* 30,000-xyz	Reserved for game application (exe)
*/
#ifdef BUILD_CORE
#define __ID_START__ 0
#else
#define __ID_START__ 30000
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
	string m_name;
	const uint16 m_id;

public:
	MClass(const char* name);

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
	* Is this class a child of the the other class
	* @param other					The class we want to check if they are our parent
	* @param trueIfIdentical		Should return true, if this class is the same as other
	* @returns If the other class is the parent of this class (Or the same, if trueIfIdentical=true)
	*/
	bool IsChildOf(const MClass* other, const bool& trueIfIdentical = true) const;


	/**
	* Getters & Setters
	*/
public:
	inline const string& GetName() const { return m_name; }
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
	const MClass* m_type;
public:
	SubClassOf() : m_type(Type::StaticClass()) { }
	SubClassOf(const MClass* type) : SubClassOf() { operator=(type); }

	inline SubClassOf<Type>& operator=(const MClass* value)
	{
		if (value != nullptr && value->IsChildOf(Type::StaticClass()))
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
	inline bool IsBaseClass() const { return m_type == Type::StaticClass(); }

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
	static const MClass* StaticClass();

	/** Return single class instance used by this class' parent type */
	static const MClass* ParentStaticClass();

	/** Return single class instance used by this class	*/
	virtual const MClass* GetClass() = 0;
};




/**
* Generates required function signatures for managed objects
*/
#define CLASS_BODY() \
	friend class ClassName ## _Class; \
public: \
	static const MClass* StaticClass(); \
	static const MClass* ParentStaticClass(); \
	virtual const MClass* GetClass(); \
private:


/**
* Generates the required MClass and function content for Managed objects
*/
#define CLASS_SOURCE(ClassName, API) \
class API ClassName ## _Class : public MClass \
{ \
private: \
	friend class ClassName; \
	ClassName m_refObj; \
	\
	ClassName ## _Class() : MClass(#ClassName) {} \
\
	virtual ManagedObject* NewObject(void* dst = nullptr) const { return dst == nullptr ? new ClassName : new(dst) ClassName; } \
\
	virtual const MClass* GetParentClass() const { return ClassName::ParentStaticClass(); } \
public: \
	const ClassName* GetReferenceObject() const { return &m_refObj; } \
}; \
\
const MClass* ClassName::StaticClass() { static ClassName ## _Class sc; return &sc; } \
const MClass* ClassName::ParentStaticClass() { return __super::StaticClass(); } \
const MClass* ClassName::GetClass() { return ClassName::StaticClass(); } 