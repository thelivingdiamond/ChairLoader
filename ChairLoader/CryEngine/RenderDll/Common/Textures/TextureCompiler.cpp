// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "TextureCompiler.h"

#if defined(CRY_ENABLE_RC_HELPER)

//////////////////////////////////////////////////////////////////////////
// Provides facilities to asynchronously invoke RC and to capture
// file changes made by invoked RCs
	#include <CryThreading/IJobManager_JobDelegator.h>

DECLARE_JOB("AsyncResourceCompiler", TAsyncResourceCompilerJob, CTextureCompiler::ConsumeQueuedResourceCompiler);

//////////////////////////////////////////////////////////////////////////
CTextureCompiler::CTextureCompiler()
{
}

CTextureCompiler::~CTextureCompiler()
{
}

//////////////////////////////////////////////////////////////////////////
CryMutex CTextureCompiler::g_RCmutex;
CTextureCompiler* CTextureCompiler::g_pRCInstance = nullptr;

void CTextureCompiler::DestroyInstance()
{
	CryAutoLock<CryMutex> rcAutoLock(g_RCmutex);
	if (g_pRCInstance)
	{
		delete g_pRCInstance;
		g_pRCInstance = nullptr;
	}
}

CTextureCompiler& CTextureCompiler::GetInstance()
{
	CryAutoLock<CryMutex> rcAutoLock(g_RCmutex);
	if (!g_pRCInstance)
	{
		g_pRCInstance = new CTextureCompiler();
		assert(g_pRCInstance != nullptr);

		atexit(DestroyInstance);
	}

	return *g_pRCInstance;
}

//////////////////////////////////////////////////////////////////////////
void CTextureCompiler::GetInputFilename(
  const char* filename,
  const unsigned int index,
  char* inputFilename,
  size_t inputFilenameSizeInBytes)
{
	if (inputFilename == 0 || inputFilenameSizeInBytes < sizeof(inputFilename[0]))
	{
		return;
	}

	const char* ext = GetExtension(filename);

	if (ext && stricmp(ext, "dds") == 0)
	{
		switch (index)
		{
		case 0:
			ReplaceExtension(filename, "tif", inputFilename, inputFilenameSizeInBytes);
			return;
		default:
			inputFilename[0] = 0;
			return;
		}
	}

	if (index > 0)
	{
		inputFilename[0] = 0;
		return;
	}

	cry_strcpy(inputFilename, inputFilenameSizeInBytes, filename);
}

//////////////////////////////////////////////////////////////////////////
// choose a specialized imposter, because otherwise file-format
// constraints for specific texture-types may be violated
	#define COMPILE_DELAYED_REGULAR "EngineAssets/TextureMsg/TextureCompiling.dds"
	#define COMPILE_DELAYED_CUBEMAP "EngineAssets/TextureMsg/TextureCompiling_cm.dds"
	#define COMPILE_DELAYED_CUBEDIF "EngineAssets/TextureMsg/TextureCompiling_cm_diff.dds"
	#define COMPILE_DELAYED_NORMAL  "EngineAssets/TextureMsg/TextureCompiling_ddn.dds"
	#define COMPILE_DELAYED_NORMALA "EngineAssets/TextureMsg/TextureCompiling_ddna.dds"

	#define COMPILE_FAILED_REGULAR  "EngineAssets/TextureMsg/RCError.dds"
	#define COMPILE_FAILED_CUBEMAP  "EngineAssets/TextureMsg/RCError_cm.dds"
	#define COMPILE_FAILED_CUBEDIF  "EngineAssets/TextureMsg/RCError_cm_diff.dds"
	#define COMPILE_FAILED_NORMAL   "EngineAssets/TextureMsg/RCError_ddn.dds"
	#define COMPILE_FAILED_NORMALA  "EngineAssets/TextureMsg/RCError_ddna.dds"

static const char* GetDelayedTexture(const char* szFile)
{
	const char* szImposter;
	szImposter = COMPILE_DELAYED_REGULAR;
	if (strstr(szFile, "_ddna.") != 0)
		szImposter = COMPILE_DELAYED_NORMALA;
	else if (strstr(szFile, "_ddn.") != 0)
		szImposter = COMPILE_DELAYED_NORMAL;
	else if (strstr(szFile, "_cm_diff.") != 0)
		szImposter = COMPILE_DELAYED_CUBEDIF;
	else if (strstr(szFile, "_cm.") != 0)
		szImposter = COMPILE_DELAYED_CUBEMAP;

	return szImposter;
}

static const char* GetFailedTexture(const char* szFile)
{
	const char* szImposter;
	szImposter = COMPILE_FAILED_REGULAR;
	if (strstr(szFile, "_ddna.") != 0)
		szImposter = COMPILE_FAILED_NORMALA;
	else if (strstr(szFile, "_ddn.") != 0)
		szImposter = COMPILE_FAILED_NORMAL;
	else if (strstr(szFile, "_cm_diff.") != 0)
		szImposter = COMPILE_FAILED_CUBEDIF;
	else if (strstr(szFile, "_cm.") != 0)
		szImposter = COMPILE_FAILED_CUBEMAP;

	return szImposter;
}

// Make the dummy destination-file date one tick older than the source-file
// to allow the compilation being re-triggered in case the editor is closed earlier.
// For error-dummies this behavior can be disabled to prevent infinite retry-loops.
	#define COMPILE_DELAYED_DELTA 1
	#define COMPILE_FAILED_DELTA  0

static bool CopyDummy(const char* szImposter, const char* szSrcFile, const char* szDstFile, __int64 timedelta)
{
	string dummyPath = szImposter;
	bool success = true;
	ICryPak::FileTime ft = 0;

	{
		FILE* pSrcFile = gEnv->pCryPak->FOpen(szSrcFile, "rb");
		if (pSrcFile)
		{
			ft = gEnv->pCryPak->GetModificationTime(pSrcFile);
			ft -= timedelta;

			gEnv->pCryPak->FClose(pSrcFile);
		}
	}

	// Copy the dummy file and make it either the same age as the previous destination file,
	// or a bit older than the source file.
	// Take care of copying dummy out of the pak onto disk.
	// Dummy then triggers recompilation in subsequent Sandbox starts as well.
	FILE* pSrcFile = gEnv->pCryPak->FOpen(dummyPath, "rb");
	HANDLE hDestFile = CreateFile(szDstFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

	success = (pSrcFile && (hDestFile != INVALID_HANDLE_VALUE));
	if (success)
	{
	#define CHUNK_SIZE 64 * 1024

		char* buf = new char[CHUNK_SIZE];
		size_t readBytes = 0;
		DWORD writtenBytes = 0;
		size_t totalBytes = 0;

		while (!gEnv->pCryPak->FEof(pSrcFile))
		{
			readBytes = gEnv->pCryPak->FReadRaw(buf, sizeof(char), CHUNK_SIZE, pSrcFile);
			success = WriteFile(hDestFile, buf, sizeof(char) * readBytes, &writtenBytes, NULL) != FALSE;

			if (!success || (readBytes != writtenBytes))
			{
				success = false;
				break;
			}

			totalBytes += readBytes;
		}

		delete[] buf;

	#undef CHUNK_SIZE

		// Prevent zero-byte files being returned as valid DDSs.
		success = success && (totalBytes != 0);
	}

	if (pSrcFile)
	{
		gEnv->pCryPak->FClose(pSrcFile);
	}

	if (hDestFile != INVALID_HANDLE_VALUE)
	{
		if (ft)
		{
			FILETIME FT;

			FT.dwLowDateTime = (DWORD)(ft & 0xFFFFFFFF);
			FT.dwHighDateTime = (DWORD)(ft >> 32);

			SetFileTime(hDestFile, 0, 0, &FT);
		}

		CloseHandle(hDestFile);
	}

	if (!success)
	{
		DeleteFile(szDstFile);
	}

	return success;
}

static bool CopyResult(const char* szSrcFile, const char* szDstFile)
{
	bool success = true;

	if (strcmp(szSrcFile, szDstFile))
	{
		success = true;

		if (GetFileAttributes(szDstFile) != INVALID_FILE_ATTRIBUTES)
		{
			success = success && (DeleteFile(szDstFile) != FALSE);
		}

		success = success && (GetFileAttributes(szSrcFile) != INVALID_FILE_ATTRIBUTES);
		success = success && (MoveFile(szSrcFile, szDstFile) != FALSE);

		if (!success)
		{
			// try spin for 1.5 seconds, there might still be a file lock on either source or target
			// compilation isn't time critical, and this occurs infrequently
			for (int i = 0; (i < 5) && (!success); ++i)
			{
				success = true;

				CrySleep(100);
				if (GetFileAttributes(szDstFile) != INVALID_FILE_ATTRIBUTES)
				{
					CrySleep(100);
					success = success && (DeleteFile(szDstFile) != FALSE);
				}

				CrySleep(100);
				success = success && (GetFileAttributes(szSrcFile) != INVALID_FILE_ATTRIBUTES);
				CrySleep(100);
				success = success && (MoveFile(szSrcFile, szDstFile) != FALSE);
			}

			if (!success)
			{
	#if defined(DEBUG) && defined(_DEBUG)
				iLog->Log("Debug: RN: from \"%s\", to \"%s\"\n", szSrcFile, szDstFile);
	#endif
			}
		}
	}

	return success;
}

/* Cases for sequence of events:
 *
 * A) tif is written to, no dds exist or dds is older
 *  1) tif-monitor is triggered
 *  2) corresponding dds is requested via the streaming system
 *  3) RCHelper is called via mfInvokeRC
 *  4) RCHelper checks dates and request enters QueueResourceCompiler
 *  5) dummy dds is copied and request put into the queue
 *   a) dds-monitor is triggered
 *  6) streamer loads dummy dds
 *
 *  7) background process compiles dds and writes it out
 *   a) dds-monitor is triggered
 *  8) task is removed from queue
 *  9) loop until queue empty
 *
 * B) tif is written to, while RC is compiling it
 *  1) access denied on tif to writer
 *   VERIFY: does RC hold a lock on the file for the whole processing? It should, to have this work
 *   TODO: the queue should be updated, the RC killed and restarted
 *
 * C) tif is written to again, before RC is compiling it (double/tripple/... saving)
 *  1) tif-monitor is triggered
 *  2) corresponding dds is requested via the streaming system
 *  3) RCHelper is called via mfInvokeRC
 *  4) RCHelper checks dates and request enters QueueResourceCompiler
 *  5) file is found in watch-list and queuing is skipped
 *
 * D) dds is written to, while RC is compiling it
 *  1) dds is overwritten by RC, or RC get's access denied for writing
 *   a) dds-monitor is triggered
 *
 * E) dds is written to, before RC is compiling it
 *  1) dds is overwritten by RC
 *   a) dds-monitor is triggered
 *
 * In case of an error a texture with an error-message is placed for the requested outfile.
 */
bool CTextureCompiler::HasQueuedResourceCompiler(const char* szSrcFile, const char* szDstFile)
{
	// check if to be generated from the same source
	m_rwLockWatch.RLock();
	TWatchItem queued = m_mWatchList.find(szDstFile);
	bool doadd = (queued == m_mWatchList.end());
	bool exists = (!doadd && (queued->second == szSrcFile));
	m_rwLockWatch.RUnlock();

	return exists;
}

CResourceCompilerHelper::ERcCallResult CTextureCompiler::QueueResourceCompiler(const char* szSrcFile, const char* szDstFile, const bool bWindow, const bool bRefresh)
{
	// check if to be generated from the same source
	if (HasQueuedResourceCompiler(szSrcFile, szDstFile))
	{
		return eRcCallResult_queued;
	}

	// replace/place source
	m_rwLockWatch.WLock();
	TWatchItem queued = m_mWatchList.find(szDstFile);
	bool doadd = (queued == m_mWatchList.end());
	bool exists = (!doadd && (queued->second == szSrcFile));

	if (exists)
	{
		m_rwLockWatch.WUnlock();
		return eRcCallResult_queued;
	}

	if (!doadd)
	{
		queued->second = szSrcFile;
	}
	else
	{
		m_mWatchList[szDstFile] = szSrcFile;

		if (CRenderer::CV_r_texturecompilingIndicator > 0)
		{
			const char* szImposter = GetDelayedTexture(szDstFile);
			if (!CopyDummy(szImposter, szSrcFile, szDstFile, COMPILE_DELAYED_DELTA))
			{
				// If this fails it's not critical, although it'll show
				// the "ReplaceMe" texture instead of a proper one.
				//	return eRcCallResult_notFound;
			}

			// Provide cubemap's diffuse texture dummy as well if it is necessary
			if (strstr(szSrcFile, "_cm."))
			{
				string szDstDiff = AddSuffix(szDstFile, "_diff");

				const char* szImposter = GetDelayedTexture(szDstDiff.c_str());
				if (!CopyDummy(szImposter, szSrcFile, szDstDiff.c_str(), COMPILE_DELAYED_DELTA))
				{
					// If this fails it's not critical, although it'll show
					// the "ReplaceMe" texture instead of a proper one.
					//	return eRcCallResult_notFound;
				}
			}
		}
	}
	m_rwLockWatch.WUnlock();

	ForkOffResourceCompiler(szSrcFile, szDstFile, bWindow, bRefresh);
	return eRcCallResult_queued;
}

void CTextureCompiler::ForkOffResourceCompiler(const char* szSrcFile, const char* szDstFile, const bool bWindow, const bool bRefresh)
{
	m_rwLockProcessing.WLock();
	bool empty = !m_qProcessingList.size();
	#if defined(STLPORT)
	m_qProcessingList.push_back(TProcItem());
	#else
	m_qProcessingList.emplace_back(TProcItem());
	#endif
	TProcItem& addedrc = m_qProcessingList.back();

	// have to be valid after the unlock
	addedrc.src = szSrcFile;
	addedrc.dst = szDstFile;
	addedrc.windowed = bWindow;
	addedrc.refresh = bRefresh;
	m_rwLockProcessing.WUnlock();

	// This approach spawns 1 new thread every time we had an empty list.
	// The thread automatically consumes the list for as long as it's not empty
	// and then terminates. When the editor is closed while the thread is running
	// it's handled by the job-system, no need to manually handle it here.
	if (empty)
	{
		TAsyncResourceCompilerJob compileJob(&addedrc);

		compileJob.SetClassInstance(&CTextureCompiler::GetInstance());
		compileJob.Run();
	}
}

void CTextureCompiler::ConsumeQueuedResourceCompiler(TProcItem* item)
{
	// no need to protect
	int pending = m_qProcessingList.size();

	m_rwLockNotify.RLock();
	std::for_each(m_sNotifyList.begin(), m_sNotifyList.end(), [=](IAsyncTextureCompileListener* notify)
	{
		notify->OnCompilationQueueTriggered(pending);
	});
	m_rwLockNotify.RUnlock();

	while (item)
	{
		{
			m_rwLockNotify.RLock();
			std::for_each(m_sNotifyList.begin(), m_sNotifyList.end(), [=](IAsyncTextureCompileListener* notify)
			{
				notify->OnCompilationStarted(item->src.c_str(), item->dst.c_str(), pending);
			});
			m_rwLockNotify.RUnlock();

			// Always use a temporary file as outfile, otherwise RC may write to the
			// file before it's even loaded as a dummy.
			string tmpsrc = AddSuffix(item->dst, "_tmp");

			iLog->Log("Compile texture from \"%s\", to \"%s\"\n", item->src.c_str(), tmpsrc.c_str());
			item->returnval = InvokeResourceCompiler(item->src.c_str(), tmpsrc.c_str(), item->windowed, true);

			// It's not a cube-map
			if (strstr(item->dst.c_str(), "_cm.") == 0)
			{
				bool bSuccess = (item->returnval == eRcExitCode_Success);
				bSuccess = bSuccess && CopyResult(tmpsrc.c_str(), item->dst.c_str());
				if (!bSuccess)
				{
					if (CRenderer::CV_r_texturecompilingIndicator >= 0)
					{
						CopyDummy(GetFailedTexture(item->dst.c_str()), item->src.c_str(), item->dst.c_str(), COMPILE_FAILED_DELTA);
					}

					DeleteFile(tmpsrc.c_str());
				}

				// Suggest reload of the texture after success or failure.
				if (gEnv && gEnv->pRenderer)
				{
					gEnv->pRenderer->EF_ReloadFile_Request(item->dst.c_str());
				}
			}
			else
			{
				string dstdiff = AddSuffix(item->dst.c_str(), "_diff");
				string tmpdiff = AddSuffix(tmpsrc, "_diff");

				bool bSuccess = (item->returnval == eRcExitCode_Success);
				bSuccess = bSuccess && CopyResult(tmpsrc.c_str(), item->dst.c_str());
				bSuccess = bSuccess && CopyResult(tmpdiff.c_str(), dstdiff.c_str());
				if (!bSuccess)
				{
					if (CRenderer::CV_r_texturecompilingIndicator >= 0)
					{
						CopyDummy(GetFailedTexture(item->dst.c_str()), item->src.c_str(), item->dst.c_str(), COMPILE_FAILED_DELTA);
						CopyDummy(GetFailedTexture(dstdiff.c_str()), item->src.c_str(), dstdiff.c_str(), COMPILE_FAILED_DELTA);
					}

					DeleteFile(tmpsrc.c_str());
					DeleteFile(tmpdiff.c_str());
				}

				// Suggest reload of the texture after success or failure.
				if (gEnv && gEnv->pRenderer)
				{
					gEnv->pRenderer->EF_ReloadFile_Request(item->dst.c_str());
					gEnv->pRenderer->EF_ReloadFile_Request(dstdiff.c_str());
				}
			}

			m_rwLockNotify.RLock();
			std::for_each(m_sNotifyList.begin(), m_sNotifyList.end(), [=](IAsyncTextureCompileListener* notify)
			{
				notify->OnCompilationFinished(item->src.c_str(), item->dst.c_str(), item->returnval);
			});
			m_rwLockNotify.RUnlock();
		}

		TProcQueue::iterator foundrc;

		m_rwLockWatch.WLock();
		m_rwLockProcessing.WLock();

		for (foundrc = m_qProcessingList.begin(); (&(*foundrc) != item) && (foundrc != m_qProcessingList.end()); ++foundrc)
			;
		if (foundrc != m_qProcessingList.end())
		{
			m_mWatchList.erase(item->dst);
			m_qProcessingList.erase(foundrc);
			if (m_qProcessingList.size())
				item = &m_qProcessingList.front();
			else
				item = NULL;
			pending = m_qProcessingList.size();
		}
		// Severe container-damage, should not happen.
		else
		{
			assert(0);
		}

		m_rwLockWatch.WUnlock();
		m_rwLockProcessing.WUnlock();
	}

	m_rwLockNotify.RLock();
	std::for_each(m_sNotifyList.begin(), m_sNotifyList.end(), [=](IAsyncTextureCompileListener* notify)
	{
		notify->OnCompilationQueueDepleted();
	});
	m_rwLockNotify.RUnlock();
}

//////////////////////////////////////////////////////////////////////////
bool CTextureCompiler::ProcessTextureIfNeeded(
  const char* originalFilename,
  char* processedFilename,
  size_t processedFilenameSizeInBytes)
{
	// allocates 1k upto 4k on the stack
	char sSrcFile[MAX_PATH];
	char sDestFile[MAX_PATH];

	char sFullSrcFilename[MAX_PATH];
	char sFullDestFilename[MAX_PATH];

	GetOutputFilename(originalFilename, sDestFile, sizeof(sDestFile));

	// Adjust filename so that it is global.
	gEnv->pCryPak->AdjustFileName(sDestFile, sFullDestFilename, 0);

	for (uint32 dwIndex = 0;; ++dwIndex)    // check for all input files
	{
		GetInputFilename(originalFilename, dwIndex, sSrcFile, sizeof(sSrcFile));

		if (sSrcFile[0] == 0)
		{
			break;          // last input file
		}

		// Adjust filename so that it is global.
		gEnv->pCryPak->AdjustFileName(sSrcFile, sFullSrcFilename, 0);

	#if defined(_RENDERER)
		// Prevent a race-condition of date-checker and file-writer
		// by doing an early queue-check (files are in the queue until
		// they are completely processed)
		if (HasQueuedResourceCompiler(sFullSrcFilename, sFullDestFilename))
		{
			break;
		}
	#endif

		// compile if there is no destination
		// compare date of destination and source , recompile if needed
		// load dds header, check hash-value of the compile settings in the dds file, recompile if needed (not done yet)

		CDebugAllowFileAccess dafa;
		FILE* pDestFile = gEnv->pCryPak->FOpen(sDestFile, "rb");
		FILE* pSrcFile = gEnv->pCryPak->FOpen(sSrcFile, "rb");
		dafa.End();

		// files from the pak file do not count as date comparison do not seem to work there
		if (pDestFile)
		{
			if (gEnv->pCryPak->IsInPak(pDestFile))
			{
				gEnv->pCryPak->FClose(pDestFile);
				pDestFile = 0;
			}
		}

		if (pSrcFile)
		{
			if (gEnv->pCryPak->IsInPak(pSrcFile))
			{
				gEnv->pCryPak->FClose(pSrcFile);
				pSrcFile = 0;
			}
		}

		bool bInvokeResourceCompiler = false;

		// is there no destination file?
		if (pSrcFile && !pDestFile)
		{
			bInvokeResourceCompiler = true;
		}

		// if both files exist, is the source file newer?
		if (pSrcFile && pDestFile)
		{
			ICryPak::FileTime timeSrc = gEnv->pCryPak->GetModificationTime(pSrcFile);
			ICryPak::FileTime timeDest = gEnv->pCryPak->GetModificationTime(pDestFile);

			// if the timestamp is identical, then it might be:
			//  1) a valid compiled target                          -> don't try
			//  2) a critically failed target (error file)          -> don't try again
			if (timeDest == (timeSrc - COMPILE_FAILED_DELTA))
			{
				bInvokeResourceCompiler = false;
			}
			// if the timestamp is one less than the source, then it might be:
			//  1) a genuine outdated target                        -> do try
			//  2) a queued target which was never run (dummy file) -> do try again
			else if (timeDest == (timeSrc - COMPILE_DELAYED_DELTA))
			{
				bInvokeResourceCompiler = true;
			}
			// otherwise, it might be
			//  1) older timestamp and the target is outdated       -> do try
			//  2) newer timestamp because the source was reverted  -> do try
			else
			{
				bInvokeResourceCompiler = (timeDest != timeSrc);
			}
		}

		if (pDestFile)
		{
			gEnv->pCryPak->FClose(pDestFile);
			pDestFile = 0;
		}

		if (pSrcFile)
		{
			gEnv->pCryPak->FClose(pSrcFile);
			pSrcFile = 0;
		}
		else
		{
			continue;
		}

		if (bInvokeResourceCompiler && gEnv->IsEditor())
		{
			// call rc.exe
			//
			// All requests are time-check already, force the RC to refresh the
			// target file in any case
	#if defined(_RENDERER)
			if (QueueResourceCompiler(sFullSrcFilename, sFullDestFilename, false, true) != eRcCallResult_queued)    // false=no window, true=force compile
	#else
			if (InvokeResourceCompiler(sFullSrcFilename, sFullDestFilename, false, true) != eRcExitCode_Success)    // false=no window, true=force compile
	#endif
			{
				cry_strcpy(processedFilename, processedFilenameSizeInBytes, originalFilename);

				// rc failed
				return false;
			}
		}

		break;
	}

	// load without using RC (e.g. TGA)
	cry_strcpy(processedFilename, processedFilenameSizeInBytes, sDestFile);

	// rc didn't fail
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CTextureCompiler::AddAsyncTextureCompileListener(IAsyncTextureCompileListener* pListener)
{
	m_rwLockNotify.WLock();
	m_sNotifyList.insert(pListener);
	m_rwLockNotify.WUnlock();
}

void CTextureCompiler::RemoveAsyncTextureCompileListener(IAsyncTextureCompileListener* pListener)
{
	m_rwLockNotify.WLock();
	m_sNotifyList.erase(pListener);
	m_rwLockNotify.WUnlock();
}

#else

//////////////////////////////////////////////////////////////////////////
void CTextureCompiler::AddAsyncTextureCompileListener(IAsyncTextureCompileListener* pListener)
{
}

void CTextureCompiler::RemoveAsyncTextureCompileListener(IAsyncTextureCompileListener* pListener)
{
}

#endif
