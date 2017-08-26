#include "../../../GameInclude.h"

namespace HERCULES_GAME{

#define TIME_UNIT 0.016666f //改變轉輪速度時用的時間單位（秒）
#define POSITIVE_ANGLE(angle) \
{\
	angle = fmodf(angle,360.0f);\
	if(angle<0) {angle += 360.0f;}\
}

	//public
	Wheel* Wheel::create(UWORD amount, float pointer, float iconOffset)
	{
		Wheel* wheel = new (std::nothrow) Wheel();
		if (wheel && wheel->InitWheel(amount, pointer, iconOffset))
		{
			wheel->autorelease();
			return wheel;
		}
		CC_SAFE_DELETE(wheel);
		return nullptr;
	}

	Wheel* Wheel::createWithFile(const std::string& filename, UWORD amount, float pointer, float iconOffset)
	{
		Wheel* wheel = new (std::nothrow) Wheel();
		if (wheel && wheel->InitWheelWithFile(filename, amount, pointer, iconOffset))
		{
			wheel->autorelease();
			return wheel;
		}
		CC_SAFE_DELETE(wheel);
		return nullptr;
	}

	Wheel* Wheel::createWithSpriteFrame(SpriteFrame *spriteFrame, UWORD amount, float pointer, float iconOffset)
	{
		Wheel* wheel = new (std::nothrow) Wheel();
		if (wheel && spriteFrame && wheel->InitWheelWithSpriteFrame(spriteFrame, amount, pointer, iconOffset))
		{
			wheel->autorelease();
			return wheel;
		}
		CC_SAFE_DELETE(wheel);
		return nullptr;
	}

	Wheel* Wheel::createWithSpriteFrameName(GDSTRING spriteFrameName, UWORD amount, float pointer, float iconOffset)
	{
		auto spriteFrame = SpriteFrameCache::getInstance()->getSpriteFrameByName(spriteFrameName);
		return createWithSpriteFrame(spriteFrame, amount, pointer, iconOffset);
	}

	void Wheel::SetIcon(Wheel_Icon* icon)
	{
		CCASSERT(icon->m_tag < m_vecIconEdge.size(), "Oversize.");
		for (auto i = 0; i < m_vecWheelIcon.size(); i++)
		{
			CCASSERT(m_vecWheelIcon.at(i)->m_tag != icon->m_tag, "Same tag is invalid");
		}

		m_vecWheelIcon.push_back(icon);

		auto rotated = icon->m_node->getRotation() + m_vecIconEdge.at(icon->m_tag) + m_icon_angle / 2;
		icon->m_node->setRotation(rotated);

		icon->m_node->setTag(icon->m_tag);
		this->addChild(icon->m_node, icon->m_tag);
	}

	Wheel::Wheel_Icon* Wheel::GetIcon(UBYTE tag)
	{
		for (auto i : m_vecWheelIcon)
		{
			if (i->m_tag == tag)
			{
				return i;
			}
		}
	}

	void Wheel::RemoveIcon(UBYTE tag)
	{
		this->removeChildByTag(tag);

		for (auto i = m_vecWheelIcon.begin(); i != m_vecWheelIcon.end(); i++)
		{
			if ((*i)->m_tag == tag)
			{
				m_vecWheelIcon.erase(i);
			}
		}
	}

	void Wheel::SetSpeed(Speed_State* speedState)
	{
		if (m_isRun == false){ m_vecSpeedState.push_back(speedState); }
	}

	void Wheel::RemoveSpeed(UBYTE order)
	{
		if (m_isRun == false)
		{
			for (auto i = m_vecSpeedState.begin(); i != m_vecSpeedState.end(); i++)
			{
				if ((*i)->m_order == order)
				{
					m_vecSpeedState.erase(i);
					break;
				}
			}
		}
	}

	UBYTE Wheel::GetPointerTo(void)
	{
		auto calculateRotated = m_pointer_angle - this->getRotation();
		SBYTE iconIndex = -1;

		POSITIVE_ANGLE(calculateRotated);

		for (auto i = 0; i < m_vecIconEdge.size(); i++)
		{
			float nextEdge;
			//當檢查的格數為最後一格時，手動調整下一格為第一格
			if (m_vecIconEdge.at(i) == m_vecIconEdge.back())
			{
				nextEdge = m_vecIconEdge.front();
			}
			else
			{
				nextEdge = m_vecIconEdge.at(i + 1);
			}

			if (calculateRotated > m_vecIconEdge.at(i))
			{
				if (calculateRotated < nextEdge)
				{
					iconIndex = i;
				}
			}
		}
		//繞了一圈還找不到的情況下，表示指針指向0度附近。例如：icon兩邊的角度是30度跟330度，而指針指向的角度是10度
		if (iconIndex == -1)
		{
			for (auto i = 0; i < m_vecIconEdge.size(); i++)
			{
				float nextEdge;
				if (m_vecIconEdge.at(i) == m_vecIconEdge.back())
				{
					nextEdge = m_vecIconEdge.front();
				}
				else
				{
					nextEdge = m_vecIconEdge.at(i + 1);
				}

				auto tempangle = nextEdge - m_vecIconEdge.at(i);

				if (tempangle < 0)
				{
					iconIndex = i;
				}
			}
		}
		return iconIndex;
	}

	void Wheel::Clear(void)
	{
		this->removeAllChildrenWithCleanup(true);
		m_vecWheelIcon.resize(0);
		m_vecSpeedState.resize(0);
	}

	void Wheel::Run(void)
	{
		sort(m_vecWheelIcon.begin(), m_vecWheelIcon.end(), [](Wheel_Icon* x, Wheel_Icon* y)->bool{return(x->m_tag < y->m_tag); });
		sort(m_vecSpeedState.begin(), m_vecSpeedState.end(), [](Speed_State* x, Speed_State* y)->bool{return(x->m_order < y->m_order); });

		if (m_vecSpeedState.empty())
		{
			CCLOG("Don't set speed yet\n");
			return;
		}
		else if (m_isRun == false)
		{
			auto speed = Speed::create(RepeatForever::create(RotateBy::create(1.0f, m_basicSpeed)), 0.0f);
			speed->setTag(1);

			m_speedScaleOffset = 0.0f;
			m_nowSpeed = nullptr;
			m_nextSpeed = m_vecSpeedState.front();
			m_veciteSpeedIndex = m_vecSpeedState.begin();
			this->schedule(CC_SCHEDULE_SELECTOR(Wheel::_OnGoing), 0, CC_REPEAT_FOREVER, 0.0f);
			this->runAction(speed);
			_ToNextSpeed(0.0f);
			m_isRun = true;
		}
	}

	bool Wheel::OnReadyToStop(void)
	{
		return m_canStop;
	}

	void Wheel::Stop(UBYTE tag, float duration, UBYTE distance, float stopSpeed)
	{
		if (duration < 0.0f){ return; }
		if (distance <= 0){ return; }

		if (m_canStop == true)
		{
			m_winTag = tag;
			m_stopSpeed = stopSpeed;
			m_stopDuration = duration;
			m_stopDistance = distance;
			m_speedScaleOffset = 0.0f;

			auto speedAction = static_cast<Speed*>(this->getActionByTag(1));
			auto nowSpeed = m_basicSpeed * speedAction->getSpeed();

			if (m_stopSpeed > nowSpeed){ m_stopSpeed = nowSpeed; }

			auto stopTime = (2 * m_stopDistance * m_icon_angle) / (m_stopSpeed);

			if (stopTime >= m_stopDuration)
			{ 
				CCLOG("Needing time > Having time"); 
			}
			else
			{
				m_stopDuration = m_stopDuration - stopTime;
			}

			this->scheduleUpdate();
			m_canStop = false;
		}
	}

	void Wheel::Stop(const function<void()>& func, UBYTE tag, float duration, UBYTE distance, float stopSpeed)
	{
		Stop(tag, duration, distance, stopSpeed);
		CallBackFunc = func;
	}

	bool Wheel::InitWheel(UWORD amount, float pointer, float iconOffset)
	{
		_InitSetting(amount, pointer, iconOffset);
		return init();
	}

	bool Wheel::InitWheelWithSpriteFrame(SpriteFrame *spriteFrame, UWORD amount, float pointer, float iconOffset)
	{
		_InitSetting(amount, pointer, iconOffset);
		return initWithSpriteFrame(spriteFrame);
	}

	bool Wheel::InitWheelWithFile(const std::string& filename, UWORD amount, float pointer, float iconOffset)
	{
		_InitSetting(amount, pointer, iconOffset);
		return initWithFile(filename);
	}

	void Wheel::update(float delta)
	{
		auto pointerTo = GetPointerTo();
		auto checkTag = (pointerTo + m_vecIconEdge.size() - m_stopDistance) % m_vecIconEdge.size();//取得距離指標特定距離的格子

		if (m_stopToWin == true)
		{
			m_stopDuration -= delta;
			if (m_stopDuration < 0.0f){ m_stopDuration = 0.0f; }

			if (m_stopDuration == 0.0f){ m_stopExtraTime -= delta; }

			if (m_stopExtraTime < 0.0f){ m_stopExtraTime = 0.0f; }
		}
		//檢查winTag的位子
		if (checkTag == m_winTag && m_stopDistance >= 0)
		{
			auto nowSpeed = m_basicSpeed * static_cast<Speed*>(this->getActionByTag(1))->getSpeed();
			auto distanceAngle = ((m_stopSpeed + nowSpeed) * m_stopDuration) / 2;
	
			if (m_stopExtraTime == 0.0f)
			{
				_StopSchedule();
				m_stopDistance--;
			}
			else
			{
				m_stopToWin = true;
				if (distanceAngle >= 360)
				{
					m_speedScaleOffset = _GetSpeedScale(nowSpeed, m_stopSpeed, m_stopDuration);
				}
				else if (m_stopDuration != 0.0f)
				{
					auto stopTime = (2 * 360.0f) / (nowSpeed + m_stopSpeed);

						if (stopTime > m_stopDuration)
						{
							m_stopExtraTime = stopTime;
							m_stopDuration = 0.0f;
						}
						else
						{
							m_stopExtraTime = 0.0f;
						}
						m_speedScaleOffset = _GetSpeedScale(nowSpeed, m_stopSpeed, stopTime);
				}

			}
		}
		//減速
		auto speedAction = static_cast<Speed*>(this->getActionByTag(1));
		auto speedScale = speedAction->getSpeed();

		speedScale += m_speedScaleOffset;
		speedAction->setSpeed(speedScale);

		if (speedScale <= 0)
		{
			m_stopExtraTime = 1.0f;
			speedAction->setSpeed(0.0f);
			m_isRun = false;
			m_stopToWin = false;
			this->unscheduleUpdate();
			this->unscheduleAllSelectors();
			this->unscheduleAllCallbacks();
			this->stopActionByTag(1);
			if (CallBackFunc != nullptr){ CallBackFunc(); }
		}
	}

	Wheel::Wheel()
		: m_isRun(false)
		, m_canStop(false)
		, m_stopToWin(false)
		, m_basicSpeed(120.0f)
		, m_stopExtraTime(1.0f)
		, m_vecWheelIcon(NULL)
		, m_vecSpeedState(NULL)
		, CallBackFunc(nullptr)
	{}

	Wheel::~Wheel(){}

	//Protected
	void Wheel::_OnGoing(float d)
	{
		//設定檢查是否還有下一段的時間
		if (this->isScheduled(CC_SCHEDULE_SELECTOR(Wheel::_ToNextSpeed)))
		{
		}
		else
		{
			m_nowSpeed = m_nextSpeed;
			auto totalTime = m_nowSpeed->m_fadeTime + m_nowSpeed->m_duration;

			if (m_nextSpeed->m_order != m_vecSpeedState.back()->m_order)
			{
				m_veciteSpeedIndex++;
				m_nextSpeed = *m_veciteSpeedIndex;
			}
			else
			{
				m_nextSpeed = nullptr;
			}
			this->scheduleOnce(CC_SCHEDULE_SELECTOR(Wheel::_ToNextSpeed), totalTime);
		}
		//執行此段變速
		auto speedAction = static_cast<Speed*>(this->getActionByTag(1));
		auto oldSpeed = speedAction->getSpeed();
		auto newSpeed = oldSpeed + m_speedScaleOffset;

		if (m_speedScaleOffset < 0 && newSpeed < m_nowSpeed->m_speed)
		{
			newSpeed = m_nowSpeed->m_speed;
		}
		else if (m_speedScaleOffset>0 && newSpeed > m_nowSpeed->m_speed)
		{
			newSpeed = m_nowSpeed->m_speed;
		}
		speedAction->setSpeed(newSpeed);

	}

	void Wheel::_ToNextSpeed(float d)
	{
		if (m_nextSpeed == nullptr)
		{
			_ReadyToStop();
			this->unscheduleAllSelectors();
		}
		else
		{
			auto speedAction = static_cast<Speed*>(this->getActionByTag(1));
			auto fadetime = m_nextSpeed->m_fadeTime / TIME_UNIT;

			if (fadetime == 0)
			{
				m_speedScaleOffset = m_nextSpeed->m_speed;
			}
			else
			{
				m_speedScaleOffset = (m_nextSpeed->m_speed - speedAction->getSpeed()) / fadetime;
			}
			
		}
	}

	void Wheel::_StopSchedule(void)
	{
		auto angleDistance = ((m_stopDistance + 1) % m_vecIconEdge.size()) * m_icon_angle;
		auto speedAction = static_cast<Speed*>(this->getActionByTag(1));
		auto nowSpeed = (speedAction->getSpeed() * m_basicSpeed) ;
		auto expectedTime = (2 * angleDistance) / nowSpeed;

		if (m_stopDistance == 0)
		{
			expectedTime = RANDOMR(expectedTime / 5, expectedTime - (expectedTime / 5));
			m_speedScaleOffset = _GetSpeedScale(nowSpeed, 0.0f, expectedTime);
		}
		else
		{
			m_speedScaleBase = _GetSpeedScale(nowSpeed, 0.0f, expectedTime);

			if (m_speedScaleBase > m_speedScaleOffset)
			{
				m_speedScaleOffset = m_speedScaleBase;
			}
		}
	}

	void Wheel::_ReadyToStop(void)
	{
		m_canStop = true;
	}

	float Wheel::_GetSpeedScale(float startSpeed, float endSpeed, float deltaTime)
	{
		if (deltaTime <= 0){ return 0.0f; }

		auto startSpeedFrames = startSpeed * TIME_UNIT;
		auto endSpeedFrames = endSpeed * TIME_UNIT;
		auto deltaFrames = deltaTime / TIME_UNIT;
		auto deltaSpeed = (endSpeedFrames - startSpeedFrames) / deltaFrames;
		auto endSpeedScale = (startSpeedFrames + deltaSpeed) / (m_basicSpeed * TIME_UNIT);
		auto nowSpeedScale = static_cast<Speed*>(this->getActionByTag(1))->getSpeed();

		return endSpeedScale - nowSpeedScale;
	}

	void Wheel::_InitSetting(UWORD amount, float pointer, float iconOffset)
	{
		m_amount = amount;
		m_icon_angle = 360.0f / amount;
		m_icon_offset = iconOffset;
		m_pointer_angle = pointer;
		m_canStop = false;
		m_isRun = false;
		m_stopToWin = false;
		m_basicSpeed = 120.0f;
		CallBackFunc = nullptr;

		m_vecSpeedState.clear();
		m_vecWheelIcon.reserve(m_amount);
		m_vecIconEdge.resize(m_amount);

		auto temp_angleOffset = m_icon_angle / 2;
		auto edgeAngle = m_icon_offset - temp_angleOffset;

		POSITIVE_ANGLE(edgeAngle);

		for (auto i = 0; i < m_vecIconEdge.size(); i++)
		{
			m_vecIconEdge.at(i) = (fmodf(edgeAngle + m_icon_angle*i, 360.0f));
		}

		auto deltaAngle = RANDOMR(0.0f, 360.0f);
		this->setRotation(deltaAngle);
	}

}