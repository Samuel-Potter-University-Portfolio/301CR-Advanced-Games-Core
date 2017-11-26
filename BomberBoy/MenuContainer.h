#pragma once
#include "Core\Core-Common.h"


/**
* Holds server UI elements, so that they can be easily switched on/off
* (In future a heirachy/panel approach to turn on/off would be better)
*/
class MenuContainer
{
private:
	bool bIsActive = true;
	std::vector<UGUIBase*> m_elements;

public:
	/** Callback for when this container is activated */
	virtual void OnActive() {}
	/** Callback for when this container is deactivated */
	virtual void OnDeactive() {}


	/**
	* Sets up a default menu look
	* @param parent			The hud to add elements to
	* @param title			The title of the menu
	* @param font			The font to use for the title
	* @param scalingMode	The method of scaling to use
	* @param size			The size of the menu
	* @param anchor			The anchor to attach the menu to
	* @param layer			The layer to put everything on
	*/
	void SetupDefaultMenu(AHUD* parent, const string& title, const sf::Font* font, const ULabel::ScalingMode& scalingMode, const vec2& size, const vec2& anchor);


	/**
	* Create an element in this container
	* @parent		The hud to create the element under
	* @type			The type of element to create
	*/
	inline UGUIBase* AddElement(AHUD* parent, SubClassOf<UGUIBase> type)
	{ 
		UGUIBase* elem = parent->AddElement(type);
		elem->SetActive(bIsActive);
		m_elements.emplace_back(elem);
		return elem;
	}

	/**
	* Create an element in this container
	* @parent		The hud to create the element under
	*/
	template<class Type>
	inline Type* AddElement(AHUD* parent)
	{
		Type* elem = parent->AddElement<Type>();
		elem->SetActive(bIsActive);
		m_elements.emplace_back(elem);
		return elem;
	}

	/**
	* Add an already existing element
	* @param elem		The element to add
	*/
	inline UGUIBase* AddElement(UGUIBase* elem)
	{
		elem->SetActive(bIsActive);
		m_elements.emplace_back(elem);
		return elem;
	}


	/**
	* Getters & Setters
	*/
public:

	inline bool IsActive() const { return bIsActive; }
	inline void SetActive(const bool& value) 
	{
		if (bIsActive == value)
			return;
		bIsActive = value;

		for (uint32 i = 0; i < m_elements.size(); ++i)
			m_elements[i]->SetActive(bIsActive);

		if (bIsActive)
			OnActive();
		else
			OnDeactive();
	}
};

