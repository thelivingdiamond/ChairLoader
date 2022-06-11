#pragma once
class ICVar
{
public:
  virtual ~ICVar() = 0;
  virtual void Release() = 0;
  virtual int GetIVal() = 0;
  virtual __int64 GetI64Val() = 0;
  virtual float GetFVal() = 0;
  virtual const char *GetString() = 0;
  virtual const char *GetDataProbeString() = 0;
  virtual void Set(const int) = 0;
  virtual void Set(const float) = 0;
  virtual void Set(const char *) = 0;
  virtual void ForceSet(const char *) = 0;
  virtual void ClearFlags(int) = 0;
  virtual int GetFlags() = 0;
  virtual int SetFlags(int) = 0;
  virtual int GetType() = 0;
  virtual const char *GetName() = 0;
  virtual const char *GetHelp() = 0;
  virtual bool IsConstCVar() = 0;
  virtual void SetOnChangeCallback(void (ICVar *)) = 0;
  virtual void AddOnChangeFunctor(const SFunctor *) = 0;
  virtual unsigned __int64 GetNumberOfOnChangeFunctors() = 0;
  virtual const SFunctor *GetOnChangeFunctor(unsigned __int64) = 0;
  virtual bool RemoveOnChangeFunctor(const unsigned __int64) = 0;
  virtual void GetOnChangeCallback()(ICVar *) = 0;
  virtual void GetMemoryUsage(ICrySizer *) = 0;
  virtual int GetRealIVal() = 0;
  virtual void DebugLog(const int, const ICVar::EConsoleLogMode) = 0;


  };
