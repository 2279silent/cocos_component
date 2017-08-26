#include "menu.h"

//------------MENU-------------//
CMenu::CMenu()
{
}

CMenu::~CMenu()
{
	_optionsList.clear();
}

bool CMenu::init(void)
{
	_menuData = new CMenu::MenuData;
	_menuData->currentOpation = nullptr;
	_menuData->holdOpation = nullptr;
	_menuData->isActivated = true;
	return true;
}

void CMenu::SetEnable(bool enable)
{
	_menuData->isActivated = enable;
}

void CMenu::AddOpation(MenuOption* option)
{
	_optionsList.push_front(option);
}

void CMenu::RemoveOption(MenuOption* option)
{
	if (_menuData->currentOpation != option)
	{
		_optionsList.remove(option);
	}
}

void CMenu::Hold(const std::string& optionName)
{
	if (_menuData->isActivated == false)
	{
		return;
	}

	for (auto& opation : _optionsList)
	{
		if (opation->GetName().compare(optionName) == 0)
		{
			_menuData->holdOpation = opation;
			opation->Hold();
		}
		else
		{
			opation->StandBy();
		}
	}
}

void CMenu::Choose(const std::string& optionName)
{
	if (_menuData->isActivated == false)
	{
		return;
	}

	for (auto& opation : _optionsList)
	{
		if (opation->_opationCallBack == nullptr)
		{
			return;
		}

		if (opation->GetName().compare(optionName) == 0)
		{
			_menuData->holdOpation = nullptr;
			_menuData->currentOpation = opation;
			opation->Choose();
		}
		else
		{
			opation->Invisible();
		}
	}
}

MenuOption* CMenu::GetCurrentOpation(void)
{
	return _menuData->currentOpation;
}

void CMenu::SetCurrentOpation(MenuOption* option)
{
	_menuData->currentOpation = option;
}

uint32_t CMenu::GetAmountOfOpation(void)
{
	auto amount = 0;

	for (auto& opation : _optionsList)
	{
		amount++;
	}
	return amount;
}
//MenuOpation
MenuOption::MenuOption()
{
	_opationCallBack = nullptr;
	_icon = nullptr;
	_state = CMenu::MenuState::STANDBY;
	//_dirty = false;
}

MenuOption::~MenuOption()
{
	_icon->release();
}

void MenuOption::SetIcon(Node* icon)
{
	if (icon == nullptr)
	{
		return;
	}

	icon->retain();
	if (_icon == nullptr)
	{		
		_icon = icon;
	}
	else
	{
		_icon->release();
		_icon = icon;
	}
	//_dirty = true;
}

void MenuOption::StandBy(void)
{
	if (_opationCallBack)
	{
		_opationCallBack(this, CMenu::MenuState::STANDBY);
	}
	_state = CMenu::MenuState::STANDBY;
}

void MenuOption::Hold(void)
{
	if (_opationCallBack)
	{
		_opationCallBack(this, CMenu::MenuState::HOLD);
	}
	_state = CMenu::MenuState::HOLD;
}

void MenuOption::Choose(void)
{
	if (_opationCallBack)
	{
		_opationCallBack(this, CMenu::MenuState::CHOOSE);
	}
	_state = CMenu::MenuState::CHOOSE;
}

void MenuOption::Invisible(void)
{
	if (_icon)
	{
		_icon->cleanup();
	}
	if (_opationCallBack)
	{
		_opationCallBack(this, CMenu::MenuState::INVISIBLE);
	}
	_state = CMenu::MenuState::INVISIBLE;
}