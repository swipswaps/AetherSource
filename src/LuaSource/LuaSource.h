#pragma once

//------------------------------------------------------------------------------
// File: PushSource.H
//
// Desc: DirectShow sample code - In-memory push mode source filter
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "ScriptEngine.h"

// UNITS = 10 ^ 7  
// UNITS / 30 = 30 fps;
// UNITS / 20 = 20 fps, etc
//const REFERENCE_TIME FPS_120 = UNITS / 120;
//const REFERENCE_TIME FPS_100 = UNITS / 100;
//const REFERENCE_TIME FPS_60 = UNITS / 60;
//const REFERENCE_TIME FPS_50 = UNITS / 50;
//const REFERENCE_TIME FPS_30 = UNITS / 30;
//const REFERENCE_TIME FPS_20 = UNITS / 20;
//const REFERENCE_TIME FPS_10 = UNITS / 10;
//const REFERENCE_TIME FPS_5  = UNITS / 5;
//const REFERENCE_TIME FPS_4  = UNITS / 4;
//const REFERENCE_TIME FPS_3  = UNITS / 3;
//const REFERENCE_TIME FPS_2  = UNITS / 2;
//const REFERENCE_TIME FPS_1  = UNITS / 1;

/**********************************************
*
*  Class declarations
*
**********************************************/

class CPin : public CSourceStream, public IAMStreamConfig, public IKsPropertySet
{
public:
	CPin(HRESULT *pHr, CSource *pFilter);
	~CPin();

	int m_iFrameNumber;

	//HRESULT OnThreadCreate(void);
	//HRESULT OnThreadDestroy(void);

	// Support multiple display formats
	HRESULT CheckMediaType(const CMediaType *pMediaType);
	HRESULT GetMediaType(int iPosition, CMediaType *pMt);

	// Set the agreed media type and set up the necessary parameters
	HRESULT SetMediaType(const CMediaType *pMediaType);

	// Override the version that offers exactly one media type
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
	HRESULT FillBuffer(IMediaSample *pSample);

	// Quality control
	// Not implemented because we aren't going in real time.
	// If the file-writing filter slows the graph down, we just do nothing, which means
	// wait until we're unblocked. No frames are ever dropped.
	STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q) { return E_FAIL;	}

	//////////////////////////////////////////////////////////////////////////
	//  IUnknown
	//////////////////////////////////////////////////////////////////////////
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	STDMETHODIMP_(ULONG) AddRef() { return GetOwner()->AddRef(); } // gets called often...
	STDMETHODIMP_(ULONG) Release() { return GetOwner()->Release(); }

	//////////////////////////////////////////////////////////////////////////
	//  IAMStreamConfig
	//////////////////////////////////////////////////////////////////////////
	HRESULT STDMETHODCALLTYPE GetFormat(AM_MEDIA_TYPE **ppMt);
	HRESULT STDMETHODCALLTYPE SetFormat(AM_MEDIA_TYPE *pMt);
	HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities(int *piCount, int *piSize);
	HRESULT STDMETHODCALLTYPE GetStreamCaps(int iIndex, AM_MEDIA_TYPE **ppMt, BYTE *pSCC);

	//////////////////////////////////////////////////////////////////////////
	//  IKsPropertySet
	//////////////////////////////////////////////////////////////////////////
	HRESULT STDMETHODCALLTYPE Get(REFGUID guidPropSet, DWORD dwPropID, void *pInstanceData, DWORD cbInstanceData, void *pPropData, DWORD cbPropData, DWORD *pcbReturned);
	HRESULT STDMETHODCALLTYPE Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, DWORD cbInstanceData, void *pPropData, DWORD cbPropData);
	HRESULT STDMETHODCALLTYPE QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);
protected:
	//int m_FramesWritten;				// To track where we are in the file
	//BOOL m_bZeroMemory;                 // Do we need to clear the buffer?
	//CRefTime m_rtSampleTime;	        // The time stamp for each sample

	//int m_iFrameNumber;

	REFERENCE_TIME m_rtFrameLength;

	RECT m_rScreen;                     // Rect containing entire screen coordinates

	//int m_iHeight;                 // The current image height
	//int m_iWidth;                  // And current image width
	//int m_iCurrentBitDepth;             // Screen bit depth
	//int m_iRepeatTime;                  // Time in msec between frames

	//CMediaType m_MediaType;
	//CCritSec m_cSharedState;            // Protects our internal state
	//CImageDisplay m_Display;            // Figures out our media type for us

	CScriptEngine *m_pScriptEngine;
};

class CFilter : public CSource
{
public:
	static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *pHr);
	
	//
	STDMETHODIMP Run(REFERENCE_TIME tStart);
	STDMETHODIMP Stop();
	STDMETHODIMP GetState(DWORD dwMilliSecsTimeout, FILTER_STATE *State);

	//STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	//ULONG STDMETHODCALLTYPE AddRef() { return CSource::AddRef(); };
	//ULONG STDMETHODCALLTYPE Release() { return CSource::Release(); };
private:
	// Constructor is private because you have to use CreateInstance
	CFilter(IUnknown *pUnk, HRESULT *pHr);
	~CFilter();
	
	CPin *m_pPin;
};