#ifndef	RESOURCE_H
#define	RESOURCE_H

#define cGameResource CGameResource::Getinstance()
#include "tinyxml2/tinyxml2.h"
#include "cocos2d.h"

USING_NS_CC;
using namespace std;

class ResourceInfo
{
public:
	ResourceInfo(const string& path = "");
	virtual ~ResourceInfo(){};

	virtual void Load(void) = 0;
	virtual void LoadAsync(void) = 0;
	virtual void UnLoad(void) = 0;
	virtual void SetFilePath(const string& path);
	virtual string GetFilePath(void);
	virtual bool IsInMemory(void) = 0;
protected:
	string _filePath;
	bool _async;
};

class ImageResource :public ResourceInfo
{
public:
	ImageResource(const string& path);
	~ImageResource();

	void Load(void)override;
	void LoadAsync(void)override;
	void UnLoad(void)override;	
	bool IsInMemory(void)override;
	bool TextureSplit(void);
	void LoadFailHandle(void);
protected:

	Texture2D* _texture;
	string _plistPath;//如果檔案沒有plist，儲存的是spriteframe的名字
	bool _split;
};

//class AudioResource :public ResourceInfo
//{
//public:
//	AudioResource(const string& path);
//	~AudioResource();
//
//	void Load(void)override;
//	void LoadAsync(void)override;
//	void UnLoad(void)override;
//	bool IsInMemory(void)override;
//
//	inline void SetKey(const string& key){ _key = key; }
//
//	inline const string& GetKey(void){ return _key; }
//protected:
//	string _key;
//};

class Segment
{
public:
	enum class Type
	{
		IMAGE,
		AUDIO,
		VIDEO,
		NONE
	};
public:
	Segment(const string name);
	~Segment();

	bool CheckFile(void);//檢查是否所有檔案都已經載入
	bool ClearFile(void);
	void SetRetain(bool retain){ _retain = retain; }
	void AddResource(ResourceInfo* info);
	void Load(bool async);
	bool UnLoad(const string& filePath);
	inline uint64_t GetFileAmount(void){ return _fileList.size(); }
	uint64_t GetLoadedFileAmount(void);

	string segmentName;
	Type type;
protected:	
	void CheckLoadProcess(float dt);

	vector<ResourceInfo*> _fileList;
	vector<ResourceInfo*>::iterator _forLoading;
	ULONG _loadedFileAmount;
	float _loadingTimeOut;
	bool _retain;
};

class ResourceThread
{
public:
	static ResourceThread* GetInstance();

	void AddToSplit(ImageResource* resource);
	void Stop(bool clear);
protected:
	static ResourceThread* _instance;

	void SplitProcess(float dt);

	deque<ImageResource*> _splitQuene;
private:
	ResourceThread();
	~ResourceThread();
};

class CGameResource
{
public:
	static CGameResource* Getinstance(void);

	bool Init(void);
	void Update(float dt);

	void SetGameLayer(Layer* layer);
	Layer* GetGameLayer(void);

	//load
	void OpenList(const string& fileName);//xml檔
	void CloseList(const string& fileName);
	void SetLoadingSize(uint32_t loadingFileSize);
	bool StartByThread(void);
	bool StartImmediately(void);
	void Stop(void);
	void Pause(void);
	void Resume(void);
	void RemoveSegment(const string& segmentName);
	void Clear(void);

	float GetLoadingProgress(vector<string>& checkList);
protected:
	CGameResource(void);
	~CGameResource(void);

	void LoadProcess(float dt);
	void LoadFailHandle(const string& failFile);

	void ParseImage(Segment* segment, tinyxml2::XMLElement* segmentNode);
	//void ParseAudio(Segment* segment, tinyxml2::XMLElement* segmentNode);
	//void ParseVideo(Segment* segment, tinyxml2::XMLElement* segmentNode);

	bool FullLoadingQuene(void);
	bool InWaitQueue(const string& name, Segment::Type type);

	static CGameResource* _gameResource;
	Layer* _gameLayer;

	Scheduler* _scheduler;

	bool _isLoading;
	string _path;
	uint32_t _loadingQueneMaxSize;
	int8_t _failCount;
	uint32_t _loadProcessIndex;

	deque<Segment*> _waitQuene;
	deque<Segment*>_loadingQuene;
	vector<Segment*>_loadedSegment;
};

#endif