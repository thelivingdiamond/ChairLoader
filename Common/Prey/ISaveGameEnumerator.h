#pragma once
class ISaveGameEnumerator
{
public:
  virtual ~ISaveGameEnumerator() = 0;
  virtual int GetSlot() = 0;
  virtual void Delete(const char *const) = 0;
  virtual void Update(const char *const) = 0;
  virtual int GetCount() = 0;
  virtual bool GetDescription(int, ISaveGameEnumerator::SGameDescription *) = 0;
  virtual _smart_ptr<ISaveGameThumbnail> *GetThumbnail(_smart_ptr<ISaveGameThumbnail> *result, const char *) = 0;
  virtual _smart_ptr<ISaveGameThumbnail> *GetThumbnail(_smart_ptr<ISaveGameThumbnail> *result, int) = 0;
  virtual void AddRef() = 0;
  virtual void Release() = 0;


  };
