// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef __CET_NETCONFIG_H__
#define __CET_NETCONFIG_H__

#pragma once

void AddEstablishedContext(IContextEstablisher* pEst, EContextViewState state, int token);
void AddDeclareWitness(IContextEstablisher* pEst, EContextViewState state);
void AddPopulateRebroadcaster(IContextEstablisher* pEst, EContextViewState state);
void AddDelegateAuthorityToClientActor(IContextEstablisher* pEst, EContextViewState state);
void AddClearPlayerIds(IContextEstablisher* pEst, EContextViewState state);

#endif
