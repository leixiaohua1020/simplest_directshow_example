/**
 * 最简单的基于DirectShow的视频播放器
 * Simplest DirectShow Player
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序是一个最简单的基于DirectShow的播放器。
 * 适合初学者学习DirectShow。
 *
 * This example is the simplest Player based on DirectShow.
 * Suitable for the beginner of DirectShow.
 */

#include "stdafx.h"

#include <dshow.h>
#include <atlconv.h>


#define OUTPUT_INFO 1

//Show Filter in FilterGpragh
int show_filters_in_filtergraph(IGraphBuilder *pGraph){
	printf("Filters in FilterGpragh=======\n");
	USES_CONVERSION;
	IEnumFilters *pFilterEnum=NULL;
	if(FAILED(pGraph->EnumFilters(&pFilterEnum))){
		pFilterEnum->Release();
		return -1;
	}
	pFilterEnum->Reset();
	IBaseFilter * filter = NULL;
	ULONG fetchCount = 0;
	//Pin Info
	while (SUCCEEDED(pFilterEnum->Next(1, &filter, &fetchCount)) && fetchCount){
		if (!filter){
			continue;
		}
		FILTER_INFO FilterInfo;
		if (FAILED(filter->QueryFilterInfo(&FilterInfo))){
			continue;
		}
		printf("[%s]\n",W2A(FilterInfo.achName));
		filter->Release();
	}
	pFilterEnum->Release();
	printf("==============================\n");
	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
	IGraphBuilder *pGraph = NULL;
    IMediaControl *pControl = NULL;
    IMediaEvent   *pEvent = NULL; 
	//Get some param--------------
	HRESULT hr1;
	IBasicVideo *pVideo=NULL;
	IBasicAudio *pAudio=NULL;
	IVideoWindow *pWindow=NULL;
	IMediaSeeking *pSeeking=NULL;
	
	
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
   //  Query Interface
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
    hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
	// RenderFile
    hr = pGraph->RenderFile(L"cuc_ieschool.mov", NULL);
	if (FAILED(hr)){
		printf("Error - Can't Render File.");
		return -1;
	}
#if OUTPUT_INFO
	//Get some information----------
	long video_w=0,video_h=0,video_bitrate=0,audio_volume=0;
	long long duration_1=0,position_1=0;
	REFTIME avgtimeperframe=0;
	float framerate=0,duration_sec=0,progress=0,position_sec=0;
	//Video
	hr1=pGraph->QueryInterface(IID_IBasicVideo, (void **)&pVideo);
	pVideo->get_VideoWidth(&video_w);
	pVideo->get_VideoHeight(&video_h);
	pVideo->get_AvgTimePerFrame(&avgtimeperframe);
	framerate=1/avgtimeperframe;
	//pVideo->get_BitRate(&video_bitrate);
	//Audio
	hr1=pGraph->QueryInterface(IID_IBasicAudio, (void **)&pAudio);
	//Mute
	//pAudio->put_Volume(-10000);
	printf("Some Information:\n");
	printf("Video Resolution:\t%dx%d\n",video_w,video_h);
	printf("Video Framerate:\t%.3f\n",framerate);
	//Window
	hr1=pGraph->QueryInterface(IID_IVideoWindow, (void **)&pWindow);
	pWindow->put_Caption(L"Simplest DirectShow Player");
	//pWindow->put_Width(480);
	//pWindow->put_Height(272);
	//Seek
	hr1=pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pSeeking);
	pSeeking->GetDuration(&duration_1);
	//time unit:100ns=0.0000001s
	duration_sec=(float)duration_1/10000000.0;
	printf("Duration:\t%.2f s\n",duration_sec);
	//pSeeking->SetPositions();
	//PlayBack Rate
	//pSeeking->SetRate(2.0);

	//Show Filter in FilterGpagh
	show_filters_in_filtergraph(pGraph);
	//----------------------
#endif

	printf("Progress Info\n");
	printf("Position\tProgress\n");
    if (SUCCEEDED(hr)){
        // Run
        hr = pControl->Run();
        if (SUCCEEDED(hr)){
			long evCode=0;
			//pEvent->WaitForCompletion(INFINITE, &evCode);
			while(evCode!=EC_COMPLETE){
				//Info
#if OUTPUT_INFO
				pSeeking->GetCurrentPosition(&position_1);
				position_sec=(float)position_1/10000000.0;
				progress=position_sec*100/duration_sec;
				printf("%7.2fs\t%5.2f%%\n",position_sec,progress);
#endif
				//1000ms
				pEvent->WaitForCompletion(1000, &evCode);
			}
        }
    }
	// Release resource
    pControl->Release();
    pEvent->Release();
    pGraph->Release();
    CoUninitialize();
	return 0;
}

