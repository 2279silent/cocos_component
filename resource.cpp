#include "resource.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#define CCSleep(t) Sleep(t)
#endif

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <unistd.h>
#define CCSleep(t)  usleep(t * 1000)  
#endif  

ResourceInfo::ResourceInfo(const string& path)
{
	auto fileUtis = FileUtils::getInstance();

	_filePath = fileUtis->fullPathForFilename(path);
	if (!fileUtis->isFileExist(_filePath))
	{
		_filePath = "";
	}
	_async = false;
}

void ResourceInfo::SetFilePath(const string& path)
{
	auto fileUtils = FileUtils::getInstance();

	_filePath = fileUtils->fullPathForFilename(path);
	if (!fileUtils->isFileExist(_filePath))
	{
		CCLOG("No file exit:%s", _filePath.c_str());
		_filePath = "";
	}
}

string ResourceInfo::GetFilePath(void)
{
	return _filePath;
}

ImageResource::ImageResource(const string& path)
	:ResourceInfo(path)
{
	_texture = nullptr;
	_split = false;
}

ImageResource::~ImageResource()
{
	if (_texture)
	{
		UnLoad();
		CCLOG("release protecting:%s", _filePath.c_str());
	}
}

void ImageResource::Load(void)
{
	if (_async == false && _texture == nullptr)
	{
		auto textureCache = Director::getInstance()->getTextureCache();
		auto pos = _filePath.find_last_of(".");

		_plistPath = _filePath;
		_plistPath.replace(pos, _plistPath.size(), ".plist");

		_texture = textureCache->addImage(_filePath);
		if (_texture)
		{
			CCLOG("Load texture is successful");
			CCLOG("File's path is : %s", _filePath.c_str());
			_texture->retain();

			TextureSplit();
		}
	}
}

void ImageResource::LoadAsync(void)
{
	if (_async == false && _texture == nullptr)
	{
		auto textureCache = TextureCache::getInstance();
		auto pos = _filePath.find_last_of(".");

		_plistPath = _filePath;
		_plistPath.replace(pos, _plistPath.size(), ".plist");

		_async = true;
		textureCache->addImageAsync(_filePath, [&](Texture2D* texture)->void
		{
			_texture = texture;
			if (_texture)
			{
				CCLOG("Load texture is successful");
				CCLOG("File's path is : %s", _filePath.c_str());
				_texture->retain();

				ResourceThread::GetInstance()->AddToSplit(this);
			}
			else
			{
				CCLOG("_texture is empty");
			}
		});	
	}
}

void ImageResource::UnLoad(void)
{
	if (_texture == nullptr )
	{
		return;
	}
	if (!_plistPath.empty())
	{
		auto spriteFrameCache = SpriteFrameCache::getInstance();

		if (_split)
		{
			spriteFrameCache->removeSpriteFramesFromFile(_plistPath);
			_split = false;
		}
		else
		{
			spriteFrameCache->removeSpriteFrameByName(_plistPath);
		}
	}

	_texture->release();
	_texture = nullptr;
}

bool ImageResource::IsInMemory(void)
{
	if (_async == false && _texture)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ImageResource::TextureSplit(void)
{
	if (_texture == nullptr)
	{
		CCLOG("texture is nullptr");
		return false;
	}
	auto spriteFrameCache = SpriteFrameCache::getInstance();

	if (FileUtils::getInstance()->isFileExist(_plistPath))
	{
		spriteFrameCache->addSpriteFramesWithFile(_plistPath, _texture);
		_split = true;
		CCLOG("Split texture : %s", _filePath.c_str());
	}
	else
	{
		auto prePos = _plistPath.find_last_of("/") + 1;
		auto lastPos = _plistPath.find_last_of(".");

		_plistPath = _plistPath.substr(prePos, lastPos - prePos);
		SpriteFrame* spriteFrame = SpriteFrame::createWithTexture(_texture, Rect(Vec2(0.0f, 0.0f), _texture->getContentSize()));

		spriteFrameCache->addSpriteFrame(spriteFrame, _plistPath);
		CCLOG("SpriteFrame name is %s", _plistPath.c_str());
	}	
	_async = false;
	return true;
}

void ImageResource::LoadFailHandle()
{	
	if (_texture == nullptr)
	{
		do
		{
			CCLOG("load fail...retry it");
			_async = false;
			TextureCache::getInstance()->unbindImageAsync(_filePath);
			Load();
		} while (TextureCache::getInstance()->getTextureForKey(_filePath) == nullptr);
	}
}

//AudioResource::AudioResource(const string& path)
//	:ResourceInfo(path)
//{
//
//}
//
//AudioResource::~AudioResource()
//{
//	if (cSound_->hasSound(_key))
//	{
//		cSound_->bUnLoadSound(_key);
//		CCLOG("release protecting:%s", _key.c_str());
//	}
//}
//
//void AudioResource::Load(void)
//{
//	if (_key.empty())
//	{
//		CCLOG("key is empty");
//	}
//	else if (FileUtils::getInstance()->isFileExist(_filePath))
//	{		
//		while (cSound_->hasSound(_key) == false)
//		{
//			cSound_->bLoadSound(_filePath, _key);
//			CCLOG("Load audio fail , try it again");
//			CCLOG("file's path is %s", _filePath.c_str());
//		}
//		CCLOG("Load audio is successful");
//	}
//	else
//	{
//		CCLOG("No file exit %s", _filePath.c_str());
//	}
//}
//
//void AudioResource::LoadAsync(void)
//{
//	Load();
//}
//
//void AudioResource::UnLoad(void)
//{
//	if (cSound_->bIsSoundPlaying(_key))
//	{
//		cSound_->stop(_key);
//	}
//	cSound_->bUnLoadSound(_key);
//}
//
//bool AudioResource::IsInMemory(void)
//{
//	return cSound_->hasSound(_key);
//}

Segment::Segment(const string name)
	:segmentName(name)
{
	_loadedFileAmount = 0;
	_loadingTimeOut = 2.0f;
	_retain = false;
}

Segment::~Segment()
{
	for (auto& file : _fileList)
	{
		delete file;
	}
}

bool Segment::CheckFile(void)
{
	for (auto& file : _fileList)
	{
		if (!file->IsInMemory())
		{
			return false;
		}
	}
	return true;
}

bool Segment::ClearFile(void)
{
	if (!_retain)
	{
		for (auto& file : _fileList)
		{
			file->UnLoad();			
		}
		TextureCache::getInstance()->removeUnusedTextures();
		return true;
	}
	else
	{
		CCLOG("Segment is retained: %s", segmentName.c_str());
		return false;
	}
}

void Segment::AddResource(ResourceInfo* info)
{
	_fileList.push_back(info);
}

void Segment::Load(bool async)
{
	if (async)
	{
		_forLoading = _fileList.begin();

		Director::getInstance()->getScheduler()->schedule(CC_CALLBACK_1(Segment::CheckLoadProcess, this), this, 0, false, "check process");
	}
	else
	{
		for (auto it = _fileList.begin(); it != _fileList.end(); it++)
		{
			(*it)->Load();
			_loadedFileAmount++;
		}
	}
}

bool Segment::UnLoad(const string& filePath)
{
	bool success = false;

	if (!_retain)
	{
		for (auto file = _fileList.begin(); file != _fileList.end(); file++)
		{
			if ((*file)->GetFilePath() == filePath)
			{
				(*file)->UnLoad();
				TextureCache::getInstance()->removeUnusedTextures();
				success = true;
				break;
			}
		}
	}
	else
	{
		CCLOG("Segment is retained: %s", segmentName.c_str());
	}
	return success;
}

uint64_t Segment::GetLoadedFileAmount(void)
{
	return _loadedFileAmount;
}

void Segment::CheckLoadProcess(float dt)
{
	for (auto index = 0; (index < 2 && _forLoading != _fileList.end()); index++)
	{
		(*_forLoading)->LoadAsync();
		_forLoading++;
	}
	for (; _loadedFileAmount < _fileList.size(); )
	{
		if (!_fileList.at(_loadedFileAmount)->IsInMemory())
		{
			_loadingTimeOut -= 0.016f;
			break;
		}
		else
		{
			_loadedFileAmount++;
			_loadingTimeOut = 2.0f;
		}
	}
	if (_loadedFileAmount == _fileList.size())
	{
		Director::getInstance()->getScheduler()->unschedule("check process", this);
	}	
	else if (_loadingTimeOut < 0)
	{
		static_cast<ImageResource*>(_fileList.at(_loadedFileAmount))->LoadFailHandle();
	}
}

ResourceThread* ResourceThread::_instance = nullptr;
ResourceThread* ResourceThread::GetInstance()
{
	if (_instance == nullptr)
	{
		_instance = new ResourceThread();
	}
	return _instance;
}

void ResourceThread::AddToSplit(ImageResource* resource)
{
	auto scheduler = Director::getInstance()->getScheduler();

	_splitQuene.push_back(resource);
	if (!scheduler->isScheduled("split process",this))
	{
		scheduler->schedule(CC_CALLBACK_1(ResourceThread::SplitProcess, this), this, 0, false, "split process");
	}
}

void ResourceThread::Stop(bool clear)
{
	Director::getInstance()->getScheduler()->unschedule("split process", this);
	if (clear)
	{
		_splitQuene.clear();
	}
}

void ResourceThread::SplitProcess(float dt)
{
	ImageResource* _resource = nullptr;

	if (_splitQuene.empty())
	{
		return;
	}
	else
	{
		_resource = _splitQuene.front();
		_splitQuene.pop_front();
		_resource->TextureSplit();
	}
}

ResourceThread::ResourceThread()
{

}

ResourceThread::~ResourceThread()
{
}

CGameResource* CGameResource::_gameResource = nullptr;
CGameResource* CGameResource::Getinstance(void)
{
	if (_gameResource == nullptr)
	{
		_gameResource = new CGameResource;
		if (_gameResource->Init())
		{
			return _gameResource;
		}
		else
		{
			delete _gameResource;
			_gameResource = nullptr;
		}
	}
	return _gameResource;
}

bool CGameResource::Init(void)
{
	_gameLayer = nullptr;
	_isLoading = false;
	_loadingQueneMaxSize = 2;
	_failCount = 0;

	return true;
}

void CGameResource::Update(float dt)
{
	if (FullLoadingQuene())
	{
		_failCount = 0;		
	}
	else if (_waitQuene.empty() && _loadingQuene.empty())
	{
		Stop();
	}
}

void CGameResource::SetGameLayer(Layer* layer)
{
	if (_gameLayer == nullptr)
	{
		_gameLayer = layer;
	}
}

Layer* CGameResource::GetGameLayer(void)
{
	return _gameLayer;
}

void CGameResource::OpenList(const string& fileName)
{
	tinyxml2::XMLDocument xmlDoc;
	auto result = xmlDoc.LoadFile(fileName.c_str());

	if (result != tinyxml2::XMLError::XML_SUCCESS)
	{
		CCLOG("Open list %s fail!!!", fileName.c_str());
		CCLOG("ERROR id=%d", result);
		return;
	}
	else if (_isLoading == true)
	{
		CCLOG("Open list %s fail!!!", fileName.c_str());
		CCLOG("Already Loading!!!");
		return;
	}
	else
	{
		auto pos = fileName.find_last_of("/");

		if (pos == string::npos)
		{
			_path = "";
		}
		else
		{
			_path = fileName.substr(0, pos + 1);
		}
	}

	CCLOG("Open list success: %s", fileName.c_str());

	auto rootNode = xmlDoc.RootElement();
	auto appendPath = rootNode->Attribute("path");
	auto segmentNode = rootNode->FirstChildElement("segment");

	if (appendPath != 0) _path.append(appendPath);

	while (segmentNode)
	{
		Segment* segment = new Segment(segmentNode->Attribute("name"));
		UWORD format;

		format = segmentNode->IntAttribute("format");
		switch (format)
		{
		case 0:
			segment->type = Segment::Type::IMAGE;
			if (!InWaitQueue(segment->segmentName, segment->type))
			{				
				ParseImage(segment, segmentNode);
			}
			break;
		case 1:
			segment->type = Segment::Type::AUDIO;
			if (!InWaitQueue(segment->segmentName, segment->type))
			{				
				//ParseAudio(segment, segmentNode);
			}
			break;
		case 2:
			segment->type = Segment::Type::VIDEO;
			if (!InWaitQueue(segment->segmentName, segment->type))
			{
				//ParseVideo(segment, segmentNode);
			}
			break;
		default:		break;
		}
		_waitQuene.push_back(segment);
		segmentNode = segmentNode->NextSiblingElement();
	}
}

void CGameResource::SetLoadingSize(uint32_t loadingFileSize)
{
	_loadingQueneMaxSize = loadingFileSize;
}

bool CGameResource::StartByThread(void)
{
	if (_isLoading)
	{
		CCLOG("Loading busily");
		return false;
	}
	else if (_waitQuene.empty())
	{
		CCLOG("WaitQuene is empty");
		return false;
	}

	if (!_scheduler->isScheduled("resource_update", this))
	{
		_scheduler->schedule(CC_CALLBACK_1(CGameResource::Update, this), this, 0, false, "resource_update");
	}
	if (!_scheduler->isScheduled("loadprocess", this))
	{
		_scheduler->schedule(CC_CALLBACK_1(CGameResource::LoadProcess, this), this, 0, false, "loadprocess");
		_loadProcessIndex = 0;
	}
	_isLoading = true;
	return true;
}

bool CGameResource::StartImmediately(void)
{
	if (_isLoading)
	{
		CCLOG("Loading busily");
		return false;
	}

	_isLoading = true;
	while (!_waitQuene.empty())
	{
		auto segment = _waitQuene.front();

		while (!segment->CheckFile())
		{
			segment->Load(false);
		}
		_loadedSegment.push_back(segment);
		_waitQuene.pop_front();
	}
	_isLoading = false;
	return true;
}

void CGameResource::Stop(void)
{
	auto schduler = Director::getInstance()->getScheduler();

	schduler->unschedule("resource_update", this);
	schduler->unschedule("loadprocess", this);
	ResourceThread::GetInstance()->Stop(true);
	_isLoading = false;
}

void CGameResource::Pause(void)
{
	_scheduler->pauseTarget(this);
}

void CGameResource::Resume(void)
{
	_scheduler->resumeTarget(this);
}

void CGameResource::RemoveSegment(const string& segmentName)
{
	vector<Segment*> retainSegment;

	for (auto loadedSegment = _loadedSegment.begin(); loadedSegment != _loadedSegment.end(); )
	{
		if ((*loadedSegment)->segmentName.compare(segmentName) == 0)
		{
			if ((*loadedSegment)->ClearFile())
			{
				delete *loadedSegment;
			}
			else
			{
				retainSegment.push_back(*loadedSegment);
			}
		}
		else
		{
			retainSegment.push_back(*loadedSegment);
		}
		loadedSegment++;
	}
	_loadedSegment.swap(retainSegment);
}

void CGameResource::Clear(void)
{
	vector<Segment*> retain;

	_gameLayer = nullptr;
	Stop();

	for (auto& segment : _waitQuene)
	{
		segment->ClearFile();
		delete segment;
	}
	for (auto& segment : _loadingQuene)
	{
		segment->ClearFile();
		delete segment;
	}
	for (auto segment = _loadedSegment.begin(); segment != _loadedSegment.end(); segment++)
	{
		if ((*segment)->ClearFile())
		{
			delete *segment;
		}
		else
		{
			retain.push_back(*segment);
		}
	}
	_waitQuene.clear();
	_loadingQuene.clear();
	_loadedSegment.swap(retain);
	for (auto file = _loadedSegment.begin(); file != _loadedSegment.end(); file++)
	{
		if (!(*file)->CheckFile())
		{
			(*file)->Load(false);
		}
	}
}

float CGameResource::GetLoadingProgress(vector<string>& checkList)
{	
	float totalFile = 0.0f;
	float loadedFile = 0.0f;

	for (auto& segment : _waitQuene)
	{
		for (auto name = checkList.begin(); name != checkList.end(); name++)
		{
			if (segment->segmentName == *name)
			{
				totalFile += segment->GetFileAmount();
			}
		}
	}
	for (auto& segment : _loadingQuene)
	{
		for (auto name = checkList.begin(); name != checkList.end(); name++)
		{
			if (segment->segmentName == *name)
			{
				totalFile += segment->GetFileAmount();
				loadedFile += segment->GetLoadedFileAmount();	
			}		
		}
	}

	for (auto& segment : _loadedSegment)
	{
		for (auto name = checkList.begin(); name != checkList.end(); name++)
		{
			if (segment->segmentName == *name)
			{
				totalFile += segment->GetFileAmount();
				loadedFile += segment->GetLoadedFileAmount();
			}	
		}
	}

	CCLOG("Loaded file: %f", loadedFile);
	CCLOG("Total file: %f", totalFile);
	if (totalFile > 0.0f)
	{
		auto process = (loadedFile / totalFile) * 100;

		//if (fileLoading && process == 100)
		//{
		//	process--;
		//}
		return process;
	}
	else
	{
		for (auto name : checkList)
		{
			CCLOG("No segment in quene %s", name.c_str());
		}
		
		CCLOG("Segment check list = ");
		for (auto checkName = checkList.begin(); checkName != checkList.end(); checkName++)
		{
			CCLOG("%s", (*checkName).c_str());
		}
		return 0;
	}
}

CGameResource::CGameResource(void)
	:_path("")
{
	_scheduler = Director::getInstance()->getScheduler();
}

CGameResource::~CGameResource(void)
{

}

void CGameResource::LoadProcess(float dt)
{
	if (_loadingQuene.empty())
	{
		return;
	}			
	
	if (_loadProcessIndex < _loadingQuene.size())
	{
		_loadingQuene.at(_loadProcessIndex)->Load(true);
		_loadProcessIndex++;
	}

	for (auto check = _loadingQuene.begin(); check != _loadingQuene.end(); check++)
	{
		if ((*check)->CheckFile())
		{
			_loadedSegment.push_back((*check));
			_loadingQuene.erase(check);
			_loadProcessIndex--;
			return;
		}
	}
}

void CGameResource::LoadFailHandle(const string& failFile)
{
	auto textureCache = TextureCache::getInstance();
	bool success = false;
	do
	{
		if (textureCache->getTextureForKey(failFile))
		{
			CCLOG("Wait texture split to spriteframe");
			success = true;
		}
		else
		{
			auto texture = textureCache->addImage(failFile);

			CCLOG("----- ReloadTexture ----- : %s", failFile.c_str());
			if (texture)
			{
				CCLOG("----- ReloadTexture ----- : %s", failFile.c_str());
				//TextureSplit(failFile);
				success = true;
			}
		}		
	} while (success == false);
}

void CGameResource::ParseImage(Segment* segment, tinyxml2::XMLElement* segmentNode)
{
	auto retainNode = segmentNode->FirstChildElement("retain");

	if (retainNode)
	{
		const string& retain = retainNode->GetText();

		if (retain == "true")
		{
			segment->SetRetain(true);
		}
		else if (retain == "false")
		{
			segment->SetRetain(false);
		}
		CCLOG("-----Retain segment: %s -----", retain);
	}

	auto listNode = segmentNode->FirstChildElement("list");
	while (listNode)
	{
		string extention(listNode->Attribute("extentions"));
		auto filenameNode = listNode->FirstChildElement("filename");
		while (filenameNode)
		{
			ImageResource* resource = new ImageResource(_path + std::string(filenameNode->GetText()) + extention);

			segment->AddResource(resource);
			filenameNode = filenameNode->NextSiblingElement();
		}
		listNode = listNode->NextSiblingElement();
	}
}

//void CGameResource::ParseAudio(Segment* segment, tinyxml2::XMLElement* segmentNode)
//{
//	auto retainNode = segmentNode->FirstChildElement("retain");
//
//	if (retainNode)
//	{
//		segment->SetRetain(cGameUtility->StrToBool(std::string(retainNode->GetText())));
//		CCLOG("-----Retain segment: %s -----", segment->segmentName.c_str());
//	}
//
//	auto listNode = segmentNode->FirstChildElement("list");
//	while (listNode)
//	{
//		string extention(listNode->Attribute("extentions"));
//		auto fileNode = listNode->FirstChildElement("file");
//		while (fileNode)
//		{
//			//取得key
//			auto key = std::string(fileNode->FirstChildElement("key")->GetText());
//			//取得檔名
//			auto filename = std::string(fileNode->FirstChildElement("filename")->GetText());
//			AudioResource* resource = new AudioResource(_path + filename + extention);
//
//			resource->SetKey(key);
//			segment->AddResource(resource);
//			fileNode = fileNode->NextSiblingElement();
//		}
//		listNode = listNode->NextSiblingElement();
//	}
//}
//
//void CGameResource::ParseVideo(Segment* segment, tinyxml2::XMLElement* segmentNode)
//{
//
//}

bool CGameResource::FullLoadingQuene(void)
{
	bool success = false;

	if (_waitQuene.empty())
	{
		//CCLOG("...WaitQuene is empty...");
		return false;
	}
	if (_loadingQuene.size() < _loadingQueneMaxSize)
	{
		for (; (_loadingQuene.size() < _loadingQueneMaxSize && !_waitQuene.empty());)
		{
			auto&& waitSegment = _waitQuene.front();

			_waitQuene.pop_front();
			_loadingQuene.push_back(waitSegment);
		}
		CCLOG("Full LoadingQuene successfully");
		success = true;
	}
	else
	{
		CCLOG("LoadingQuene has fulled");
		success = false;
	}
	return success;
}

bool CGameResource::InWaitQueue(const string& name, Segment::Type type)
{
	for (auto& data : _waitQuene)
	{
		if (data->segmentName.compare(name) == 0 && data->type == type)
		{
			return true;
		}
	}
	return false;
}