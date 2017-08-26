#ifndef DEF_MENU_H
#define DEF_MENU_H

#include "cocos2d.h"
#include <forward_list>

class MenuOption;

USING_NS_CC;
using namespace std;

class CMenu
{
public:
	enum class MenuState
	{
		STANDBY,
		HOLD,
		CHOOSE,
		INVISIBLE
	};
	virtual bool init(void);

	virtual void SetEnable(bool enable);
	virtual void OpenMenuPage(void) = 0;
	virtual void CloseMenuPage(void) = 0;
	virtual void AddOpation(MenuOption* option);
	virtual void RemoveOption(MenuOption* option);

	virtual void Hold(const std::string& optionName);
	virtual void Choose(const std::string& optionName);

	virtual MenuOption* GetCurrentOpation(void);
	virtual uint32_t GetAmountOfOpation(void);

	//單機使用, 不防呆
	virtual void SetCurrentOpation(MenuOption* option);
protected:
	typedef struct
	{
		MenuOption* currentOpation;
		MenuOption* holdOpation;
		bool isActivated;
	}MenuData;

	CMenu();
	~CMenu();

	MenuData* _menuData;
	forward_list<MenuOption*>_optionsList;
};

//@ class MenuOption
//使用在Menu上的選項
class MenuOption
{
public:
	MenuOption();
	~MenuOption();

	void SetIcon(Node* icon);
	inline Node* GetIcon(void){ return _icon; }
	inline void SetName(const string& name){ _name = name; }
	inline string GetName(void){ return _name; }
	//inline bool GetDirtyFlag(){ return _dirty; }
	//inline void ClearDirtyFlag(){ _dirty = false; }

	virtual void StandBy(void);
	virtual void Hold(void);
	virtual void Choose(void);
	virtual void Invisible(void);

	function<void(MenuOption* opation, CMenu::MenuState state)>_opationCallBack;
protected:
	Node* _icon;
	string _name;
	CMenu::MenuState _state;
	//bool _dirty;
};
#endif