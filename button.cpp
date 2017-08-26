#include "button.h"

//Added by BJ4
Button* Button::create(BUTTONTYPE listenKey, const string& normal, const string& light, const string& dark)
{
	auto button = new Button;
	if (button&&button->Init(listenKey, normal, light, dark))
	{
		button->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(button);
	}
	return button;
}

Button* Button::create(BUTTONTYPE listenKey, const Rect& rect)
{
	auto button = new Button;
	if (button&&button->Init(listenKey, rect))
	{
		button->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(button);
	}
	return button;
}

bool Button::Init(BUTTONTYPE listenKey, const string& normalName, const string& lightName, const string& darkName)
{
	if (!Sprite::init())
	{
		return false;
	}
	
	this->setCascadeOpacityEnabled(true);
	if (!normalName.empty())
	{
		normal = normalName;
	}
	if (!lightName.empty())
	{
		light = lightName;
	}
	if (!darkName.empty())
	{
		dark = darkName;
	}

	if (!normal.empty())
	{
		this->ChangeButtonState(BUTTONSTATE::NORMAL);
	}

	isPressed = false;
	isRelease = false;
	isAutoState = true;
	_enable = true;
	_triggerEvent = EventType::NOEVENT;

	SetListenKey(listenKey);
	return true;
}

bool Button::Init(BUTTONTYPE listenKey, const Rect& rect)
{
	if (!Sprite::init())
	{
		return false;
	}

	this->initWithTexture(nullptr, Rect(Vec2(0.0f,0.0f), rect.size));
	this->setPosition(rect.origin);
	this->setOpacity(0);
	_state = BUTTONSTATE::NORMAL;

	isPressed = false;
	isRelease = false;
	isAutoState = true;
	_enable = true;
	SetListenKey(listenKey);
	return true;
}

void Button::setVisible(bool visible)
{
	Sprite::setVisible(visible);
	SetWorking(visible);
}

void Button::setVisibleAndWork(bool visible, bool workSameAsVisible)
{
	Sprite::setVisible(visible);
	if (workSameAsVisible)
	{
		SetWorking(visible);
	}
	else
	{
		SetWorking(!visible);
	}
}

void Button::ChangeButtonState(BUTTONSTATE buttonState)
{
	switch (buttonState)
	{
	case BUTTONSTATE::NORMAL:	
		if (!normal.empty())
		{ 
			this->setSpriteFrame(normal);
			_state = buttonState;
		}		
		break;
	case BUTTONSTATE::LIGHT:	
		if (!light.empty())
		{ 
			this->setSpriteFrame(light); 
			_state = buttonState;
		}		
		break;
	case BUTTONSTATE::DARK:	
		if (!dark.empty())
		{ 
			this->setSpriteFrame(dark); 
			_state = buttonState;
		}		
		break;
	case BUTTONSTATE::MAX:		
		Sprite::init();	
		_state = buttonState;
		break;
	}
}

void Button::Refresh(void)
{
	this->ChangeButtonState(_state);
}

void Button::SetListenKey(BUTTONTYPE listenKey)
{
	vector < BUTTONTYPE > empty;

	this->listenKey.swap(empty);
	this->listenKey.push_back(listenKey);
}

void Button::AddListenKey(BUTTONTYPE listenKey)
{
	this->listenKey.push_back(listenKey);
}

void Button::SetWorking(bool isWorking)
{
	if (isWorking == _enable)
	{
		return;
	}

	_enable = isWorking;
	if (isAutoState == true)
	{
		if (isWorking == true)
		{
			ChangeButtonState(BUTTONSTATE::NORMAL);
		}
		else
		{
			ChangeButtonState(BUTTONSTATE::DARK);
		}
	}
	isPressed = false;
	isRelease = false;
	_triggerEvent = EventType::NOEVENT;
}

bool Button::IsPressed(void)
{
	bool isTrigger = isPressed;

	isPressed = false;
	return isTrigger;
}

bool Button::IsReleased(void)
{
	bool isTrigger = isRelease;

	isRelease = false;

	return isTrigger;
}

Button::Button(void)
	: touchCallback(nullptr)
	, buttonCallback(nullptr)
{
	this->setCascadeOpacityEnabled(true);
	listenKey.reserve(2);
	onPressed = bind(&Button::_OnPressed,this,placeholders::_1);
	onReleased = bind(&Button::_OnReleased, this, placeholders::_1);
	_state = BUTTONSTATE::MAX;
}

Button::~Button(void)
{
}

void Button::_OnPressed(EventType event)
{
	if (isPressed == true )
	{
		return;
	}

	if (isAutoState == true)
	{
		ChangeButtonState(BUTTONSTATE::LIGHT);
	}
	_triggerEvent = event;
	switch (event)	
	{
	case EventType::TOUCH:
		if (touchCallback != nullptr)
		{
			touchCallback(true);
		}
		break;
	case EventType::BUTTON:
		if (buttonCallback != nullptr)
		{
			buttonCallback(true);
		}
		break;
	}

	isPressed = true;
	isRelease = false;
}

void Button::_OnReleased(EventType event)
{
	if (isAutoState == true)
	{
		ChangeButtonState(BUTTONSTATE::NORMAL);
	}
	_triggerEvent = event;
	switch (event)
	{
	case EventType::TOUCH:
		if (touchCallback != nullptr)
		{
			touchCallback(false);
		}
		break;
	case EventType::BUTTON:
		if (buttonCallback != nullptr)
		{
			buttonCallback(false);
		}
		break;
	}
	isPressed = false;
	isRelease = true;
}

RePushTypeButton* RePushTypeButton::create(BUTTONTYPE listenKey, const string& normal , const string& light , const string& dark )
{
	auto button = new RePushTypeButton;

	if (button&&button->Init(listenKey, normal, light, dark))
	{
		button->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(button);
	}
	return button;
}

RePushTypeButton* RePushTypeButton::create(BUTTONTYPE listenKey, const Rect& rect)
{
	auto button = new RePushTypeButton;

	if (button&&button->Init(listenKey, rect))
	{
		button->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(button);
	}
	return button;
}

void RePushTypeButton::SetWorking(bool isWorking)
{
	if (isWorking == _enable)
	{
		return;
	}

	_enable = isWorking;

	if (isAutoState == true)
	{
		if (isWorking == true)
		{
			ChangeButtonState(BUTTONSTATE::NORMAL);
		}
		else
		{
			ChangeButtonState(BUTTONSTATE::DARK);
		}
	}
	this->unschedule("repush");
	isPressed = false;
	isRelease = false;
	_triggerEvent = EventType::NOEVENT;
}

void RePushTypeButton::_OnPressed(EventType event)
{
	if (isPressed == true && this->isScheduled("repush") == false)
	{
		return;
	}

	if (isAutoState == true)
	{
		ChangeButtonState(BUTTONSTATE::LIGHT);
	}
	_triggerEvent = event;
	switch (event)
	{
	case EventType::TOUCH:
		if (touchCallback != nullptr)
		{
			touchCallback(true);
		}
		break;
	case EventType::BUTTON:
		if (buttonCallback != nullptr)
		{
			buttonCallback(true);
		}
		break;
	}

	if (this->isScheduled("repush") == false)
	{
		this->schedule([=](float d)->void{
			onPressed(_triggerEvent);
			CCLOG("simulate repush OnPressed");
		}, _interval, CC_REPEAT_FOREVER, _delay, "repush");//模擬按鍵一直被按下
	}

	isPressed = true;
	isRelease = false;
}

void RePushTypeButton::_OnReleased(EventType event)
{
	if (this->isScheduled("repush") == true)
	{
		this->unschedule("repush");
	}

	if (isAutoState == true)
	{
		ChangeButtonState(BUTTONSTATE::NORMAL);
	}
	_triggerEvent = event;
	switch (event)
	{
	case EventType::TOUCH:
		if (touchCallback != nullptr)
		{
			touchCallback(false);
		}
		break;
	case EventType::BUTTON:
		if (buttonCallback != nullptr)
		{
			buttonCallback(false);
		}
		break;
	}
	isPressed = false;
	isRelease = true;
}

RePushTypeButton::RePushTypeButton()
{
	_interval = 0.08f;
	_delay = 0.32f;
}

RePushTypeButton::~RePushTypeButton()
{

}

TouchTypeButton* TouchTypeButton::create(BUTTONTYPE listenKey, const string& normal, const string& light, const string& dark)
{
	auto button = new TouchTypeButton;

	if (button&&button->Init(listenKey, normal, light, dark))
	{
		button->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(button);
	}
	return button;
}

TouchTypeButton* TouchTypeButton::create(BUTTONTYPE listenKey, const Rect& rect)
{
	auto button = new TouchTypeButton;

	if (button&&button->Init(listenKey, rect))
	{
		button->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(button);
	}
	return button;
}

bool TouchTypeButton::IsPressed(void)
{
	bool isTrigger = isPressed;

	if (!_freeze)
	{
		isPressed = false;
	}
	return isTrigger;
}

bool TouchTypeButton::IsReleased(void)
{
	bool isTrigger = isRelease;

	if (!_freeze)
	{
		isRelease = false;
	}
	return isTrigger;
}

void TouchTypeButton::SetWorking(bool isWorking)
{
	_work = isWorking;

	if (isAutoState == true)
	{
		if (_work == true)
		{
			ChangeButtonState(BUTTONSTATE::NORMAL);
		}
		else
		{
			ChangeButtonState(BUTTONSTATE::DARK);
		}
	}
	if (_work == false)
	{
		this->pause();
		isPressed = false;
		isRelease = false;
		_freeze = false;
	}
	else
	{
		this->resume();
	}
}

void TouchTypeButton::Freeze(void)
{
	_freeze = true;
	this->getScheduler()->pauseTarget(this);
}

void TouchTypeButton::Reset(void)
{
	isPressed = false;
	isRelease = false;
	_freeze = false;
	isAutoState = true;
	_triggerEvent = EventType::NOEVENT;
	if (this->isScheduled("touch") == true)
	{
		this->unschedule("touch");
	}
}

void TouchTypeButton::_OnPressed(EventType event)
{
	if (_freeze)
	{
		_freeze = false;
		this->getScheduler()->resumeTarget(this);
	}
	
	if (_work == true)
	{
		if (isAutoState == true)
		{
			ChangeButtonState(BUTTONSTATE::LIGHT);
		}
		_triggerEvent = event;
		switch (event)
		{
		case EventType::TOUCH:
			if (touchCallback != nullptr)
			{
				touchCallback(true);
			}
			break;
		case EventType::BUTTON:
			if (buttonCallback != nullptr)
			{
				buttonCallback(true);
			}
			break;
		}
		isPressed = true;
		isRelease = false;
	}

	if (this->isScheduled("touch") == false)
	{
		this->schedule([=](float d)->void{
			onPressed(_triggerEvent);
			CCLOG("simulate touch OnPressed");
		}, "touch");//模擬按鍵一直被按下
	}
}

void TouchTypeButton::_OnReleased(EventType event)
{
	if (_freeze)
	{
		_freeze = false;
		this->getScheduler()->resumeTarget(this);
	}

	if (this->isScheduled("touch") == true)
	{
		this->unschedule("touch");
	}

	if (_work == true)
	{
		if (isAutoState == true)
		{
			ChangeButtonState(BUTTONSTATE::NORMAL);
		}
		_triggerEvent = event;
		switch (event)
		{
		case EventType::TOUCH:
			if (touchCallback != nullptr)
			{
				touchCallback(false);
			}
			break;
		case EventType::BUTTON:
			if (buttonCallback != nullptr)
			{
				buttonCallback(false);
			}
			break;
		}
		isPressed = false;
		isRelease = true;
	}
}

TouchTypeButton::TouchTypeButton()
{
	_work = true;
	_freeze = false;
}

TouchTypeButton::~TouchTypeButton()
{

}