// Header file automatically created from a PDB.

#pragma once

class ICrySizer;

// Header: MadeUp
// _unknown/SEntityCollHist.h
struct SEntityCollHist // Id=8004C21 Size=64
{
	SEntityCollHist *pnext;
	float velImpact;
	float velSlide2;
	float velRoll2;
	int imatImpact[2];
	int imatSlide[2];
	int imatRoll[2];
	float timeRolling;
	float timeNotRolling;
	float rollTimeout;
	float slideTimeout;
	float mass;
	
#if 0
	void GetMemoryUsage(ICrySizer *arg0) const;
#endif
};

