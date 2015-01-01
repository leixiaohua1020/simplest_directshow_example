/**
 * 最简单的基于DirectShow的视频播放器（Custom）
 * Simplest DirectShow Player (Custom)
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序是一个简单的基于DirectShow的视频播放器。该播放器通过逐个添加
 * 滤镜并连接这些滤镜实现了视频的播放。适合初学者学习DirectShow。
 * 
 * This software is a simple video player based on DirectShow.
 * It Add DirectShow Filter Manually and Link the Pins of these filters
 * to play videos.Suitable for the beginner of DirectShow.
 */

#include "stdafx.h"
#include <dshow.h>
//'1':Add filters manually
//'0':Add filters automatically
#define ADD_MANUAL 1

//Find unconnect pins
HRESULT get_unconnected_pin(
	IBaseFilter *pFilter, // Pointer to the filter.
	PIN_DIRECTION PinDir, // Direction of the pin to find.
	IPin **ppPin) // Receives a pointer to the pin.
{
	*ppPin = 0;
	IEnumPins *pEnum = 0;
	IPin *pPin = 0;
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		return hr;
	}
	while (pEnum->Next(1, &pPin, NULL) == S_OK)
	{
		PIN_DIRECTION ThisPinDir;
		pPin->QueryDirection(&ThisPinDir);
		if (ThisPinDir == PinDir)
		{
			IPin *pTmp = 0;
			hr = pPin->ConnectedTo(&pTmp);
			if (SUCCEEDED(hr)) // Already connected, not the pin we want.
			{
				pTmp->Release();
			}
			else // Unconnected, the pin we want.
			{
				pEnum->Release();
				*ppPin = pPin;
				return S_OK;
			}
		}
		pPin->Release();
	}
	pEnum->Release();
	// Did not find a matching pin.
	return E_FAIL;
}

//Connect 2 filters
HRESULT connect_filters(
	IGraphBuilder *pGraph, 
	IBaseFilter *pSrc, 
	IBaseFilter *pDest)
{
	if ((pGraph == NULL) || (pSrc == NULL) || (pDest == NULL))
	{
		return E_POINTER;
	}
	//Find Output pin in source filter
	IPin *pOut = 0;
	HRESULT hr = NULL;
	hr=get_unconnected_pin(pSrc, PINDIR_OUTPUT, &pOut);
	if (FAILED(hr)){
		return hr;
	}
	//Find Input pin in destination filter
	IPin *pIn = 0;
	hr = get_unconnected_pin(pDest, PINDIR_INPUT, &pIn);
	if (FAILED(hr)){
		return hr;
	}
	//Connnect them
	hr = pGraph->Connect(pOut, pIn);
	pIn->Release();
	pOut->Release();
	return hr;
}

int _tmain(int argc, _TCHAR* argv[])
{
	IGraphBuilder *pGraph = NULL;
    IMediaControl *pControl = NULL;
    IMediaEvent   *pEvent = NULL; 
    // Init COM
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)){
        printf("Error - Can't init COM.");
        return -1;
    }

	// Create FilterGraph
   hr=CoCreateInstance(CLSID_FilterGraph, NULL,CLSCTX_INPROC_SERVER,IID_IGraphBuilder, (void **)&pGraph);
    if (FAILED(hr)){
        printf("Error - Can't create Filter Graph.");
        return -1;
    }
   // Query Interface
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
    hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);

	//1. Add Filters=======================
	//Source
	IBaseFilter *pF_source = 0;
	hr = CoCreateInstance(CLSID_AsyncReader, 0, CLSCTX_INPROC_SERVER,IID_IBaseFilter, (void**)(&pF_source));
	if (FAILED(hr)){
		printf("Failed to create File Source.\n");
		return -1;
	}
	hr = pGraph->AddFilter(pF_source, L"Lei's Source");
	if (FAILED(hr)){
		printf("Failed to add File Source to Filter Graph.\n");
		return -1;
	}
	IFileSourceFilter* pFileSource;
	pF_source->QueryInterface(IID_IFileSourceFilter, (void**)&pFileSource);
	pFileSource->Load(L"cuc_ieschool.mpg", NULL);
	pFileSource->Release();

#if ADD_MANUAL
	//Demuxer
	IBaseFilter *pF_demuxer = 0;
	hr = CoCreateInstance(CLSID_MPEG1Splitter, 0, CLSCTX_INPROC_SERVER,IID_IBaseFilter, (void**)(&pF_demuxer));
	if (FAILED(hr)){
		printf("Failed to create Demuxer.\n");
		return -1;
	}
	hr = pGraph->AddFilter(pF_demuxer, L"Lei's Demuxer");
	if (FAILED(hr)){
		printf("Failed to add Demuxer to Filter Graph.\n");
		return -1;
	}
	//Decoder
	IBaseFilter *pF_decoder = 0;
	hr = CoCreateInstance(CLSID_CMpegVideoCodec, 0, CLSCTX_INPROC_SERVER,IID_IBaseFilter, (void**)(&pF_decoder));
	if (FAILED(hr)){
		printf("Failed to create Decoder.\n");
		return -1;
	}
	hr = pGraph->AddFilter(pF_decoder, L"Lei's Decoder");
	if (FAILED(hr)){
		printf("Failed to add Decoder to Filter Graph.\n");
		return -1;
	}
	//Render
	IBaseFilter *pF_render = 0;
	hr = CoCreateInstance(CLSID_VideoRenderer, 0, CLSCTX_INPROC_SERVER,IID_IBaseFilter, (void**)(&pF_render));
	if (FAILED(hr)){
		printf("Failed to create Video Render.\n");
		return -1;
	}
	hr = pGraph->AddFilter(pF_render, L"Lei's Render");
	if (FAILED(hr)){
		printf("Failed to add Video Render to Filter Graph.\n");
		return -1;
	}
	//2. Connect Filters=======================
	hr = connect_filters(pGraph, pF_source, pF_demuxer);
	if (FAILED(hr)){
		printf("Failed to link Source and Demuxer.\n");
		return -1;
	}
	hr = connect_filters(pGraph, pF_demuxer, pF_decoder);
	if (FAILED(hr)){
		printf("Failed to link Demuxer and Decoder.\n");
		return -1;
	}
	hr = connect_filters(pGraph, pF_decoder, pF_render);
	if (FAILED(hr)){
		printf("Failed to link Decoder and Render.\n");
		return -1;
	}

	pF_source->Release();
	pF_demuxer->Release();
	pF_decoder->Release();
	pF_render->Release();
#else
	IPin*	 Pin;
	ULONG	 fetched;
	//	get output pin
	IEnumPins* pEnumPins;
	hr = pF_source->EnumPins(&pEnumPins);
	hr = pEnumPins->Reset();
	hr = pEnumPins->Next(1, &Pin, &fetched);
	pEnumPins->Release();
	//	render pin, graph builder automatically complete rest works
	hr = pGraph->Render(Pin);
#endif

    if (SUCCEEDED(hr)){
        // Run
        hr = pControl->Run();
        if (SUCCEEDED(hr)){
			long evCode=0;
			pEvent->WaitForCompletion(INFINITE, &evCode);
        }
    }
	//Release
    pControl->Release();
    pEvent->Release();
    pGraph->Release();
    CoUninitialize();
	return 0;
}



