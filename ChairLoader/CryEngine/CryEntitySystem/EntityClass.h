// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:   EntityClass.h
//  Version:     v1.00
//  Created:     18/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __EntityClass_h__
#define __EntityClass_h__
#pragma once

#include <CryEntitySystem/IEntityClass.h>

//////////////////////////////////////////////////////////////////////////
// Description:
//    Standard implementation of the IEntityClass interface.
//////////////////////////////////////////////////////////////////////////
class CEntityClass : public IEntityClass
{
public:
	CEntityClass();
	virtual ~CEntityClass();

	//////////////////////////////////////////////////////////////////////////
	// IEntityClass interface implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual void                         Release() override                { delete this; };

	virtual uint32                       GetFlags() const override         { return m_nFlags; };
	virtual void                         SetFlags(uint32 nFlags) override  { m_nFlags = nFlags; };

	virtual const char*                  GetName() const override          { return m_sName.c_str(); }
	virtual const char*                  GetScriptFile() const override    { return m_sScriptFile.c_str(); }

	virtual IEntityScript*               GetIEntityScript() const override { return m_pEntityScript; }
	virtual IScriptTable*                GetScriptTable() const override;
	virtual bool                         LoadScript(bool bForceReload) override;
	virtual UserProxyCreateFunc          GetUserProxyCreateFunc() const override { return m_pfnUserProxyCreate; };
	virtual void*                        GetUserProxyData() const override       { return m_pUserProxyUserData; };

	virtual IEntityPropertyHandler*      GetPropertyHandler() const override;
	virtual IEntityEventHandler*         GetEventHandler() const override;
	virtual IEntityScriptFileHandler*    GetScriptFileHandler() const override;

	virtual const SEditorClassInfo&      GetEditorClassInfo() const override;
	virtual void                         SetEditorClassInfo(const SEditorClassInfo& editorClassInfo) override;

	virtual int                          GetEventCount() override;
	virtual IEntityClass::SEventInfo     GetEventInfo(int nIndex) override;
	virtual bool                         FindEventInfo(const char* sEvent, SEventInfo& event) override;

	virtual TEntityAttributeArray&       GetClassAttributes() override;
	virtual const TEntityAttributeArray& GetClassAttributes() const override;
	virtual TEntityAttributeArray&       GetEntityAttributes() override;
	virtual const TEntityAttributeArray& GetEntityAttributes() const override;

	//////////////////////////////////////////////////////////////////////////

	void SetName(const char* sName);
	void SetScriptFile(const char* sScriptFile);
	void SetEntityScript(IEntityScript* pScript);

	void SetUserProxyCreateFunc(UserProxyCreateFunc pFunc, void* pUserData = NULL);
	void SetPropertyHandler(IEntityPropertyHandler* pPropertyHandler);
	void SetEventHandler(IEntityEventHandler* pEventHandler);
	void SetScriptFileHandler(IEntityScriptFileHandler* pScriptFileHandler);
	void SetEntityAttributes(const TEntityAttributeArray& attributes);
	void SetClassAttributes(const TEntityAttributeArray& attributes);

	void GetMemoryUsage(ICrySizer* pSizer) const override
	{
		pSizer->AddObject(m_sName);
		pSizer->AddObject(m_sScriptFile);
		pSizer->AddObject(m_pEntityScript);
		pSizer->AddObject(m_pPropertyHandler);
		pSizer->AddObject(m_pEventHandler);
		pSizer->AddObject(m_pScriptFileHandler);
	}
private:
	uint32                    m_nFlags;
	string                    m_sName;
	string                    m_sScriptFile;
	IEntityScript*            m_pEntityScript;

	UserProxyCreateFunc       m_pfnUserProxyCreate;
	void*                     m_pUserProxyUserData;

	bool                      m_bScriptLoaded;

	IEntityPropertyHandler*   m_pPropertyHandler;
	IEntityEventHandler*      m_pEventHandler;
	IEntityScriptFileHandler* m_pScriptFileHandler;

	SEditorClassInfo          m_EditorClassInfo;

	TEntityAttributeArray     m_entityAttributes;
	TEntityAttributeArray     m_classAttributes;
};

#endif // __EntityClass_h__
