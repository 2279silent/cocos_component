#include <chrono>
#include "aniamtionlabel.h"

AnimationLetterDefinition::AnimationLetterDefinition()
{
	_unit = 0.0f;
}

AnimationLetterDefinition::~AnimationLetterDefinition()
{

}

void AnimationLetterDefinition::addSpriteFrame(SpriteFrame* spriteFrame)
{
	_fontFrames.pushBack(spriteFrame);
}

void AnimationLetterDefinition::setTextureID(Texture2D* texture, ssize_t ID)
{
	auto iteValue = _textureIDMap.find(texture);

	if (_textureIDMap.find(texture) == _textureIDMap.end())
	{
		_textureIDMap.insert(pair<Texture2D*, ssize_t>(texture, ID));
	}
	else if (iteValue->second != ID)
	{
		iteValue->second = ID;
	}
}

bool AnimationLetterDefinition::getTextureID(Texture2D* texture, ssize_t& ID)
{
	auto iteValue = _textureIDMap.find(texture);

	if (iteValue != _textureIDMap.end())
	{
		ID = iteValue->second;
		return true;
	}
	else
	{
		return false;
	}
}

uint16_t AnimationLetterDefinition::getSpriteFrameIndex(float t)
{
	float frameIndex = floor(t / _unit);

	frameIndex = fmod(frameIndex, _fontFrames.size());
	return frameIndex;
}

FontAnimation* FontAnimation::create(void)
{
	FontAnimation* font = new FontAnimation();

	if (font)
	{
		font->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(font);
	}
	return font;
}

ssize_t FontAnimation::addTexture(Texture2D* texture)
{
	bool exist = false;
	ssize_t tempIndex = 0;

	for (auto checkTexture : _atlasTextures)
	{
		if (checkTexture == texture)
		{
			exist = true;
			break;
		}
		tempIndex++;
	}

	if (!exist)
	{
		texture->retain();
		_atlasTextures.push_back(texture);
	}

	return tempIndex;
}

void FontAnimation::addFontAnimationDefinition(uint16_t charID, AnimationLetterDefinition &animationDefinition)
{
	auto tempAnimationDefinition = _animationDefinitions.find(charID);

	if (tempAnimationDefinition != _animationDefinitions.end())
	{
		tempAnimationDefinition->second = animationDefinition;
	}
	else
	{
		_animationDefinitions.insert(pair<uint16_t, AnimationLetterDefinition>(charID, animationDefinition));
	}
}

bool FontAnimation::getFontAnimationDefinition(uint16_t charID, AnimationLetterDefinition &letterDefinition)
{
	auto tempAnimationDefinition = _animationDefinitions.find(charID);

	if (tempAnimationDefinition != _animationDefinitions.end())
	{
		letterDefinition = tempAnimationDefinition->second;
		return true;
	}
	else
	{
		return false;
	}
}

FontAnimation::FontAnimation(void)
{

}

FontAnimation::~FontAnimation()
{
	for (auto& item : _atlasTextures)
	{
		item->release();
	}
}

AnimationLabel* AnimationLabel::create(void)
{
	auto ref = new AnimationLabel();

	if (ref)
	{
		ref->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ref);
	}

	return ref;
}

AnimationLabel* AnimationLabel::create(FontAnimation* fontAnimation)
{
	auto ref = new AnimationLabel();

	if (ref && ref->setFontAnimation(fontAnimation))
	{
		ref->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(ref);
	}

	return ref;
}

bool AnimationLabel::setFontAnimation(FontAnimation* fontAnimation)
{
	if (_fontAnimation == fontAnimation)
	{
		return false;
	}

	if (_fontAnimation)
	{
		_batchNodes.clear();
		_quadCommands.clear();
		CC_SAFE_RELEASE_NULL(_fontAnimation);
	}

	_fontAnimation = fontAnimation;
	_fontAnimation->retain();
	_contentDirty = true;

	if (_reusedLetter == nullptr)
	{
		_reusedLetter = Sprite::create();
		_reusedLetter->setOpacityModifyRGB(_isOpacityModifyRGB);
		_reusedLetter->retain();
		_reusedLetter->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	}

	updateShaderProgram();

	return true;
}

void AnimationLabel::setAnimationMode(bool mode)
{
	if (_animationMode == mode)
	{
		return;
	}

	_animationMode = mode;
	resetElapsedTime();
}

void AnimationLabel::resetElapsedTime(void)
{
	_elapsedTime = chrono::duration<float>(0);
	_animationStartTime = chrono::system_clock::now();
}

void AnimationLabel::setAdditionalKerning(float space)
{
	if (_additionalKerning != space)
	{
		_additionalKerning = space;
		_contentDirty = true;
	}
}

void AnimationLabel::updateContent()
{
	alignText();
	_contentDirty = false;
}

void AnimationLabel::setOpacityModifyRGB(bool isOpacityModifyRGB)
{
	if (isOpacityModifyRGB != _isOpacityModifyRGB)
	{
		_isOpacityModifyRGB = isOpacityModifyRGB;
		updateColor();
	}
}

std::string AnimationLabel::getDescription() const
{
	char tmp[50];
	sprintf(tmp, "<AnimationLabel | Tag = %d, AnimationLabel = >", _tag);
	std::string ret = tmp;
	ret += _text;

	return ret;
}

const Size& AnimationLabel::getContentSize() const
{
	if (_contentDirty)
	{
		const_cast<AnimationLabel*>(this)->updateContent();
	}
	return _contentSize;
}

Rect AnimationLabel::getBoundingBox() const
{
	const_cast<AnimationLabel*>(this)->getContentSize();

	return Node::getBoundingBox();
}

void AnimationLabel::visit(Renderer *renderer, const Mat4 &parentTransform, uint32_t parentFlags)
{
	if (!_visible || (_text.empty() && _children.empty()))
	{
		return;
	}

	if (_animationMode)
	{
		recordElapsedTime();
		updateFrames();
	}

	if (_contentDirty)
	{
		updateContent();
	}

	uint32_t flags = processParentFlags(parentTransform, parentFlags);
	bool visibleByCamera = isVisitableByVisitingCamera();

	if (_children.empty() && !visibleByCamera)
	{
		return;
	}

	// IMPORTANT:
	// To ease the migration to v3.0, we still support the Mat4 stack,
	// but it is deprecated and your code should not rely on it
	_director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, _modelViewTransform);

	if (!_children.empty())
	{
		sortAllChildren();

		int i = 0;
		// draw children zOrder < 0
		for (; i < _children.size(); i++)
		{
			auto node = _children.at(i);

			if (node && node->getLocalZOrder() < 0)
				node->visit(renderer, _modelViewTransform, flags);
			else
				break;
		}

		if (!_text.empty())
		{
			this->draw(renderer, _modelViewTransform, flags);
		}

		for (auto it = _children.cbegin() + i; it != _children.cend(); ++it)
		{
			(*it)->visit(renderer, _modelViewTransform, flags);
		}

	}
	else if (!_text.empty())
	{
		this->draw(renderer, _modelViewTransform, flags);
	}

	_director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}

void AnimationLabel::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	if (_batchNodes.empty() || _lengthOfString <= 0)
	{
		return;
	}
	// Don't do calculate the culling if the transform was not updated
	bool transformUpdated = flags & FLAGS_TRANSFORM_DIRTY;
#if CC_USE_CULLING
	auto visitingCamera = Camera::getVisitingCamera();
	auto defaultCamera = Camera::getDefaultCamera();
	if (visitingCamera == defaultCamera) {
		_insideBounds = (transformUpdated || visitingCamera->isViewProjectionUpdated()) ? renderer->checkVisibility(transform, _contentSize) : _insideBounds;
	}
	else
	{
		_insideBounds = renderer->checkVisibility(transform, _contentSize);
	}

	if (_insideBounds)
#endif
	{
		for (auto& batchNode : _batchNodes)
		{
			auto textureAtlas = batchNode->getTextureAtlas();

			if (textureAtlas->getTotalQuads() != 0)
			{
				auto iteQuadCommand = _quadCommands.find(textureAtlas);

				if (iteQuadCommand == _quadCommands.end())
				{
					iteQuadCommand = _quadCommands.emplace(make_pair(textureAtlas, QuadCommand())).first;
				}

				iteQuadCommand->second.init(_globalZOrder, textureAtlas->getTexture()->getName(), getGLProgramState(),
					_blendFunc, textureAtlas->getQuads(), textureAtlas->getTotalQuads(), transform, flags);
				renderer->addCommand(&iteQuadCommand->second);
			}
		}
	}
}

void AnimationLabel::setString(const std::string &label)
{
	if (_text.compare(label))
	{
		_text = label;
		_lengthOfString = static_cast<int>(_text.length());
		_contentDirty = true;
	}
}

const string& AnimationLabel::getString() const
{
	return _text;
}

void AnimationLabel::setBlendFunc(const BlendFunc &blendFunc)
{
	_blendFunc = blendFunc;
	_blendFuncDirty = true;
}

const BlendFunc &AnimationLabel::getBlendFunc() const
{
	return _blendFunc;
}

void AnimationLabel::updateFrames(void)
{
	int textLen = getStringLength();
	AnimationLetterDefinition letterDef;

	for (auto& letterInfo : _lettersInfo)
	{
		if (_fontAnimation->getFontAnimationDefinition(letterInfo.utf16Char, letterDef) && letterDef.getSpriteFrames().empty() == false)
		{
			uint16_t frameIndex = letterDef.getSpriteFrameIndex(_elapsedTime.count());

			if (_loop >= 0)
			{
				if (_elapsedTime.count() > (letterDef.getDuration()*_loop))
				{
					frameIndex = _staticFrameIndex;
					setAnimationMode(false);
					_contentDirty = true;
					break;
				}
			}
			if (frameIndex != letterInfo.frameIndex)
			{
				_contentDirty = true;
				break;
			}
		}
	}
}

bool AnimationLabel::multilineTextWrapByChar()
{
	int textLen = getStringLength();
	float nextLetterX = 0.f;
	float nextLetterY = 0.f;
	float letterRight = 0.f;
	float letterTop = 0.f;
	auto contentScaleFactor = CC_CONTENT_SCALE_FACTOR();
	AnimationLetterDefinition letterDef;
	Vec2 letterPosition;

	for (int index = 0; index < textLen; index++)
	{
		auto& character = _text.at(index);

		if (_fontAnimation->getFontAnimationDefinition(character, letterDef) == false)
		{
			recordPlaceholderInfo(index, character);
			CCLOG("LabelAnimationFormatter error:can't find label-animation definition in font file for letter: %c", character);
			continue;
		}
		else if (letterDef.getSpriteFrames().empty() == true)
		{
			recordPlaceholderInfo(index, character);
			CCLOG("Frame of LabelAnimationFormatter error:can't find frame definition in FontAnimationDefinition for letter: %c", character);
			continue;
		}

		uint16_t frameIndex = letterDef.getSpriteFrameIndex(_elapsedTime.count());

		if (_elapsedTime.count() == 0.0f)
		{
			frameIndex = _staticFrameIndex;
		}
		else if (_loop >= 0)
		{
			if (_elapsedTime.count() > (letterDef.getDuration()*_loop))
			{
				frameIndex = letterDef.getSpriteFrames().size() - 1;
			}
		}

		SpriteFrame* spriteFrame = letterDef.getSpriteFrames().at(frameIndex);
		Size frameSize = spriteFrame->getOriginalSizeInPixels();

		letterPosition.x = (nextLetterX + spriteFrame->getOffsetInPixels().x) / contentScaleFactor;
		letterPosition.y = (nextLetterY - spriteFrame->getOffsetInPixels().y) / contentScaleFactor;
		recordLetterInfo(letterPosition, character, index, frameIndex);

		nextLetterX += (frameSize.width + _additionalKerning);

		letterRight = letterPosition.x + frameSize.width;
		letterTop = letterPosition.y + frameSize.height;
	}

	setContentSize(Size(letterRight, letterTop));

	return true;
}

void AnimationLabel::alignText()
{
	if (_fontAnimation == nullptr || _text.empty())
	{
		setContentSize(Size::ZERO);
		return;
	}

	auto& textures = _fontAnimation->getTextures();

	if (textures.size() > _batchNodes.size())
	{
		for (auto index = _batchNodes.size(); index < textures.size(); ++index)
		{
			auto batchNode = SpriteBatchNode::createWithTexture(textures.at(index));
			if (batchNode)
			{
				_isOpacityModifyRGB = batchNode->getTexture()->hasPremultipliedAlpha();
				_blendFunc = batchNode->getBlendFunc();
				batchNode->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
				batchNode->setPosition(Vec2::ZERO);
				_batchNodes.pushBack(batchNode);
			}
		}
	}
	if (_batchNodes.empty())
	{
		return;
	}

	multilineTextWrapByChar();

	updateQuads();

	updateColor();
}

void AnimationLabel::updateQuads()
{
	for (auto&& batchNode : _batchNodes)
	{
		batchNode->getTextureAtlas()->removeAllQuads();
	}

	for (int ctr = 0; ctr < _lengthOfString; ++ctr)
	{
		if (_lettersInfo.at(ctr).valid)
		{
			AnimationLetterDefinition letterDef;

			_fontAnimation->getFontAnimationDefinition(_lettersInfo.at(ctr).utf16Char, letterDef);

			SpriteFrame* spriteFrame = letterDef.getSpriteFrames().at(_lettersInfo.at(ctr).frameIndex);
			ssize_t textureID;

			if (letterDef.getTextureID(spriteFrame->getTexture(), textureID))
			{
				auto index = static_cast<int>(_batchNodes.at(textureID)->getTextureAtlas()->getTotalQuads());

				if (index == 0 && ctr != 0)
				{
					CCLOG("something wrong");
				}
				_reusedLetter->setBatchNode(_batchNodes.at(textureID));
				_reusedLetter->setSpriteFrame(spriteFrame);
				_reusedLetter->setPosition(_lettersInfo[ctr].positionX, _lettersInfo[ctr].positionY);
				_lettersInfo[ctr].atlasIndex = index;
				_batchNodes.at(textureID)->insertQuadFromSprite(_reusedLetter, index);
			}
		}
	}
}

void AnimationLabel::updateColor()
{
	if (_batchNodes.empty())
	{
		return;
	}

	Color4B color4(_displayedColor.r, _displayedColor.g, _displayedColor.b, _displayedOpacity);

	// special opacity for premultiplied textures
	if (_isOpacityModifyRGB)
	{
		color4.r *= _displayedOpacity / 255.0f;
		color4.g *= _displayedOpacity / 255.0f;
		color4.b *= _displayedOpacity / 255.0f;
	}

	cocos2d::TextureAtlas* textureAtlas;
	V3F_C4B_T2F_Quad *quads;
	for (auto&& batchNode : _batchNodes)
	{
		textureAtlas = batchNode->getTextureAtlas();
		quads = textureAtlas->getQuads();
		auto count = textureAtlas->getTotalQuads();

		for (int index = 0; index < count; ++index)
		{
			quads[index].bl.colors = color4;
			quads[index].br.colors = color4;
			quads[index].tl.colors = color4;
			quads[index].tr.colors = color4;
			textureAtlas->updateQuad(&quads[index], index);
		}
	}
}

void AnimationLabel::updateShaderProgram()
{
	setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP));
}

void AnimationLabel::recordElapsedTime(void)
{
	const auto& timeNow = chrono::system_clock::now();
	chrono::duration<float> differentiation = timeNow - _animationStartTime;

	_animationStartTime = timeNow;

#if COCOS2D_DEBUG
	// If we are debugging our code, prevent big delta time
	if (differentiation.count() > 0.2f)
	{
		differentiation = chrono::duration<float>(1 / 60.0f);
	}
#endif

	_elapsedTime += differentiation;
}

void AnimationLabel::reset()
{
	Node::removeAllChildrenWithCleanup(true);
	CC_SAFE_RELEASE_NULL(_reusedLetter);
	_batchNodes.clear();
	_lettersInfo.clear();

	if (_fontAnimation)
	{
		CC_SAFE_RELEASE_NULL(_fontAnimation);
	}

	setAnimationMode(false);

	setFrameIndex(0);
	setLoop(-1);

	_contentDirty = false;
	_lengthOfString = 0;
	_text.clear();

	_additionalKerning = 0.f;

	setColor(Color3B::WHITE);

	_blendFuncDirty = false;
	_blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
	_isOpacityModifyRGB = false;
	_insideBounds = true;
}

void AnimationLabel::recordLetterInfo(const cocos2d::Vec2& point, char16_t utf16Char, int letterIndex, uint16_t frameIndex)
{
	if (static_cast<vector<LetterInfo>::size_type>(letterIndex) >= _lettersInfo.size())
	{
		LetterInfo tmpInfo;
		_lettersInfo.push_back(tmpInfo);
	}
	_lettersInfo[letterIndex].utf16Char = utf16Char;
	_lettersInfo[letterIndex].frameIndex = frameIndex;
	_lettersInfo[letterIndex].valid = true;
	_lettersInfo[letterIndex].positionX = point.x;
	_lettersInfo[letterIndex].positionY = point.y;
}

void AnimationLabel::recordPlaceholderInfo(int letterIndex, char16_t utf16Char)
{
	if (static_cast<vector<LetterInfo>::size_type>(letterIndex) >= _lettersInfo.size())
	{
		LetterInfo tmpInfo;
		_lettersInfo.push_back(tmpInfo);
	}
	_lettersInfo[letterIndex].utf16Char = utf16Char;
	_lettersInfo[letterIndex].valid = false;
}

AnimationLabel::AnimationLabel()
	:_fontAnimation(nullptr)
	, _reusedLetter(nullptr)
{
	reset();
}

AnimationLabel::~AnimationLabel()
{
	if (_fontAnimation)
	{
		CC_SAFE_RELEASE_NULL(_fontAnimation);
		CC_SAFE_RELEASE_NULL(_reusedLetter);
	}
}