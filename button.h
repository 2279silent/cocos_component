#ifndef GAMELIB_KEY_H
#define GAMELIB_KEY_H

#define cKey_			CGameLibKey::GetInstance()

#include <forward_list>
#include "cocos2d.h"

USING_NS_CC;
using namespace std;

enum class BUTTONTYPE
{
	//define key
	KEY_ENTER,
	KEY_SPACE,
};

enum class WORKTYPE
{
	PUSH,
	REPUSH
};

enum class BUTTONSTATE //按鈕狀態
{
	NORMAL,
	LIGHT,
	DARK,
	MAX
};

enum class EventType
{
	TOUCH,
	BUTTON,
	NOEVENT
};

class Button:public Sprite
{
public:
	friend class CGameLibKey;
public:
	//前三個參數是給SpriteFrameName
	static Button* create(BUTTONTYPE listenKey, const string& normal = "", const string& light = "", const string& dark = "");
	//產生空白的範圍，需注意圖層
	static Button* create(BUTTONTYPE listenKey, const Rect& rect);

	virtual bool Init(BUTTONTYPE listenKey, const string& normalName, const string& lightName, const string& darkName);
	virtual bool Init(BUTTONTYPE listenKey, const Rect& rect);

	virtual void setVisible(bool visible)override;
	virtual void setVisibleAndWork(bool visible, bool workSameAsVisbile);
	virtual void ChangeButtonState(BUTTONSTATE buttonState);
	virtual void Refresh(void);

	virtual void SetListenKey(BUTTONTYPE listenKey);
	virtual void AddListenKey(BUTTONTYPE listenKey);
	virtual void SetWorking(bool isWorking);
	inline void SetAutoState(bool isAutoState){ this->isAutoState = isAutoState; }

	inline const vector<BUTTONTYPE>& GetListenKey(void){ return listenKey; }
	inline EventType GetTriggerEvent(void){ return _triggerEvent; }

	virtual bool IsPressed(void);
	virtual bool IsReleased(void);

	//Signal:true		Pressed
	//Signal:false	Released
	function<void(bool Signal)>touchCallback;
	function<void(bool Signal)>buttonCallback;
protected:

	Button();
	virtual ~Button();

	virtual void _OnPressed(EventType event);
	virtual void _OnReleased(EventType event);

	vector<BUTTONTYPE> listenKey;//對應的實體key
	
	bool isPressed;
	bool isRelease;
	bool isAutoState;
	bool _enable;

	string normal;
	string light;
	string dark;
	BUTTONSTATE _lastState;
	BUTTONSTATE _state;

	EventType _triggerEvent;
	function<void(EventType)>onPressed;//不要覆蓋這個函式
	function<void(EventType)>onReleased;//不要覆蓋這個函式
};

class RePushTypeButton :public Button
{
public:
	//前三個參數是給SpriteFrameName
	static RePushTypeButton* create(BUTTONTYPE listenKey, const string& normal = "", const string& light = "", const string& dark = "");
	//產生空白的範圍，需注意圖層
	static RePushTypeButton* create(BUTTONTYPE listenKey, const Rect& rect);

	inline void SetDelayTime(float delay){ _delay = delay; }
	inline void SetInterval(float interval){ _interval = interval; }
	void SetWorking(bool isWorking)override;
protected:
	void _OnPressed(EventType event)override;
	void _OnReleased(EventType event)override;
private:
	RePushTypeButton();
	~RePushTypeButton();

	float _interval;
	float _delay;
};

class TouchTypeButton:public Button
{
public:
	//前三個參數是給SpriteFrameName
	static TouchTypeButton* create(BUTTONTYPE listenKey, const string& normal = "", const string& light = "", const string& dark = "");
	//產生空白的範圍，需注意圖層
	static TouchTypeButton* create(BUTTONTYPE listenKey, const Rect& rect);

	virtual bool IsPressed(void)override;
	virtual bool IsReleased(void)override;

	void SetWorking(bool isWorking)override;
	void Freeze(void);
	void Reset(void);
protected:
	void _OnPressed(EventType event)override;
	void _OnReleased(EventType event)override;
private:
	TouchTypeButton();
	~TouchTypeButton();

	bool _work;
	bool _freeze;
};

#endif
