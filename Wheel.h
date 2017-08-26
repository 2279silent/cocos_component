#ifndef	WHEEL_H
#define	WHEEL_H

namespace HERCULES_GAME{
class Wheel :public cocos2d::Sprite
{
public:
	struct Wheel_Icon
	{
		Node* m_node;
		UBYTE m_tag;
		SWORD m_score;
		Wheel_Icon()
			: m_tag(0)
			, m_score(0)
			, m_node(nullptr)
		{}
	};

	struct Speed_State
	{
		UBYTE m_order;
		float m_speed;
		float m_fadeTime;//變化到這個階段的時間
		float m_duration;//這個速度要持續的時間
		Speed_State()
			: m_order(0)
			, m_speed(0.0f)
			, m_fadeTime(0.0f)
			, m_duration(0.0f)
		{}
	};

	static Wheel* create(UWORD amount, float pointer, float iconOffset = 0);
	static Wheel* createWithFile(const std::string& filename, UWORD amount, float pointer, float iconOffset = 0);
	static Wheel* createWithSpriteFrame(SpriteFrame *spriteFrame, UWORD amount, float pointer, float iconOffset = 0);
	static Wheel* createWithSpriteFrameName(GDSTRING spriteFrameName, UWORD amount, float pointer, float iconOffset = 0);

	virtual void SetIcon(Wheel_Icon* icon);
	virtual Wheel_Icon* GetIcon(UBYTE tag);
	virtual void RemoveIcon(UBYTE tag);
	virtual void SetSpeed(Speed_State* speedState);
	virtual void RemoveSpeed(UBYTE order);
	virtual UBYTE GetPointerTo(void);
	virtual void Clear(void);

	virtual void Run(void);
	virtual bool OnReadyToStop(void);
	virtual void Stop(UBYTE tag, float duration, UBYTE distance, float stopSpeed = 10.0f);
	virtual void Stop(const function<void()>& func, UBYTE tag, float duration, UBYTE distance, float stopSpeed = 10.0f);	

	virtual bool InitWheel(UWORD amount, float pointer, float iconOffset = 0);
	virtual bool InitWheelWithSpriteFrame(SpriteFrame *spriteFrame, UWORD amount, float pointer, float iconOffset = 0);
	virtual bool InitWheelWithFile(const std::string& filename, UWORD amount, float pointer, float iconOffset = 0);
	virtual void update(float delta) override;
	Wheel(void);
	virtual~Wheel(void);

protected:

	void _OnGoing(float d);
	void _ToNextSpeed(float d);
	void _StopSchedule(void);
	void _ReadyToStop(void);
	float _GetSpeedScale(float startSpeed, float endSpeed, float deltaTime);
	void _InitSetting(UWORD amount, float pointer, float iconOffset);

	UBYTE m_amount;//icon的數量
	float m_icon_angle;//每個icon是幾度
	float m_icon_offset;//icon在轉輪上的調整角度
	float m_pointer_angle;//指針在轉輪上的位子

	Speed_State* m_nextSpeed;
	Speed_State* m_nowSpeed;
	float m_speedScaleOffset;
	float m_speedScaleBase;

	UBYTE m_winTag;
	bool m_canStop;
	bool m_isRun;
	bool m_stopToWin;

	float m_basicSpeed;//預設每秒120度
	float m_stopSpeed;

	UBYTE m_stopFadeTime;//在停止階段，平均每格的減速次數
	SBYTE m_stopDistance;//在停止階段，目標距離指針的距離
	float m_stopDuration;//在停止階段，轉輪停止的時間
	float m_stopExtraTime;

	vector<Wheel_Icon*> m_vecWheelIcon;
	vector<Speed_State*> m_vecSpeedState;
	vector<Speed_State*>::iterator m_veciteSpeedIndex;
	vector<float> m_vecIconEdge;

	std::function<void()> CallBackFunc;
};
}
#endif