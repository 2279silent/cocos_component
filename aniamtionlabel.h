#ifndef ANIMATIONLABEL
#define ANIMATIONLABEL

#include<vector>
#include<unordered_map>
#include"cocos2d.h"

USING_NS_CC;
using namespace std;

class AnimationLetterDefinition
{
public:
	AnimationLetterDefinition();
	virtual ~AnimationLetterDefinition();

	void addSpriteFrame(SpriteFrame* spriteFrame);
	inline const Vector<SpriteFrame*>& getSpriteFrames(void) const { return _fontFrames; }

	void setTextureID(Texture2D* texture, ssize_t ID);
	bool getTextureID(Texture2D* texture, ssize_t& ID);

	inline void setDelayUnit(float unit) { _unit = unit; }
	float getDelayUnit(void) { return _unit; }

	inline float getDuration(void) { return _fontFrames.size()*_unit; }

	uint16_t getSpriteFrameIndex(float t);
protected:
	Vector<SpriteFrame*> _fontFrames;
	//textureID用來在FontAnimation's _atlasTextures裡找到正確的texture
	unordered_map<Texture2D*, ssize_t> _textureIDMap;
	float _unit;
};

class FontAnimation : public Ref
{
public:
	friend class AnimationLabel;
public:
	static FontAnimation* create(void);

	ssize_t addTexture(Texture2D* texture);
	inline const vector<Texture2D*>& getTextures() const { return _atlasTextures; }

	void addFontAnimationDefinition(uint16_t charID, AnimationLetterDefinition &animationDefinition);
	bool getFontAnimationDefinition(uint16_t charID, AnimationLetterDefinition &letterDefinition);
protected:
	vector<Texture2D*> _atlasTextures;
	unordered_map<uint16_t, AnimationLetterDefinition> _animationDefinitions;
private:
	FontAnimation(void);
	virtual ~FontAnimation();
};

class AnimationLabel :public Node, public LabelProtocol, public BlendProtocol
{
public:
	static AnimationLabel* create(void);
	static AnimationLabel* create(FontAnimation* fontAnimation);

	bool setFontAnimation(FontAnimation* fontAnimation);
	void setAnimationMode(bool mode);
	void resetElapsedTime(void);
	inline void setFrameIndex(uint16_t index) { _staticFrameIndex = index; }
	inline void setLoop(int16_t loop) { _loop = loop; }

	inline int getStringLength() { return _lengthOfString; }
	void setAdditionalKerning(float space);
	inline float getAdditionalKerning() const { return _additionalKerning; }

	/** Update content immediately.*/
	virtual void updateContent();

	//override Node
	virtual inline bool isOpacityModifyRGB() const override { return _isOpacityModifyRGB; }
	virtual void setOpacityModifyRGB(bool isOpacityModifyRGB) override;

	virtual std::string getDescription() const override;

	virtual const Size& getContentSize() const override;
	virtual Rect getBoundingBox() const override;

	virtual void visit(Renderer *renderer, const Mat4 &parentTransform, uint32_t parentFlags) override;
	virtual void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags) override;

	//override LabelProtocol
	virtual void setString(const std::string &label)override;
	virtual const std::string& getString() const override;

	//override BlendProtocol
	virtual void setBlendFunc(const BlendFunc &blendFunc)override;
	virtual const BlendFunc &getBlendFunc() const override;
protected:
	struct LetterInfo
	{
		char16_t utf16Char;
		uint16_t frameIndex;
		bool valid;
		float positionX;
		float positionY;
		int atlasIndex;
	};

	virtual void updateFrames(void);

	bool multilineTextWrapByChar();

	virtual void alignText();

	void updateQuads();
	//override Node
	virtual void updateColor() override;

	virtual void updateShaderProgram();

	virtual void recordElapsedTime(void);

	void reset();

	void recordLetterInfo(const cocos2d::Vec2& point, char16_t utf16Char, int letterIndex, uint16_t frameIndex);
	void recordPlaceholderInfo(int letterIndex, char16_t utf16Char);

	bool _contentDirty;

	string _text;

	FontAnimation* _fontAnimation;
	Vector<SpriteBatchNode*> _batchNodes;
	std::vector<LetterInfo> _lettersInfo;

	//used for animation

	bool _animationMode;
	uint16_t _staticFrameIndex;
	int16_t _loop;
	chrono::system_clock::time_point _animationStartTime;
	chrono::duration<float> _elapsedTime;

	//! used for optimization
	Sprite *_reusedLetter;
	Rect _reusedRect;
	int _lengthOfString;

	//layout relevant properties.
	float _additionalKerning;

	unordered_map<TextureAtlas*, QuadCommand> _quadCommands;

	bool _blendFuncDirty;
	BlendFunc _blendFunc;

	/// whether or not the label was inside bounds the previous frame
	bool _insideBounds;
	bool _isOpacityModifyRGB;
private:
	AnimationLabel();
	virtual ~AnimationLabel();
};

#endif // !ANIMATIONLABEL
