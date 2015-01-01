/**
 * 最简单的Directshow信息显示例子
 * Simplest DirectShow Info
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序是一段获取DirectShow滤镜信息的代码。通过本代码可以获得
 * DirectShow滤镜信息。适合初学者学习DirectShow。
 * 
 * This code can be used to get Directshow Filter's information.
 * Suitable for the beginner of DirectShow.
 */

#include "stdafx.h"
#include <dshow.h>
#include <atlconv.h>

char* GuidToString(const GUID &guid)
{
	char buf[64] = {0};
	_snprintf_s(
		buf,
		sizeof(buf),
		"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
	return buf;
}


int _tmain(int argc, _TCHAR* argv[])
{
	USES_CONVERSION;
	// Init COM
	HRESULT hr=NULL;
	hr= CoInitialize(NULL);
	if (FAILED(hr)){
		printf("Error, Can not init COM.");
		return -1;
	}
	printf("Directshow Filters ===============\n");
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr)){
		return hr;
	}

	IEnumMoniker *pEnumCat = NULL;
	//Category	
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoCompressorCategory, &pEnumCat, 0);
	//hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
	//hr = pSysDevEnum->CreateClassEnumerator(CLSID_MidiRendererCategory, &pEnumCat, 0);
	//hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioCompressorCategory, &pEnumCat, 0);
	//hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEnumCat, 0);
	//hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioRendererCategory, &pEnumCat, 0);

	if (hr != S_OK) {
		pSysDevEnum->Release();
		return -1;
	}
	
	IMoniker *pMoniker = NULL;
	ULONG cFetched;
	//Filter
	while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK){
		IPropertyBag *pPropBag;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,(void **)&pPropBag);
		if (FAILED(hr)){
			pMoniker->Release();
			continue;
		}
		VARIANT varName;
		VariantInit(&varName);
		hr = pPropBag->Read(L"FriendlyName", &varName, 0);
		//"FriendlyName": The name of the device.
		//"Description": A description of the device.
		//"DevicePath": A unique string that identifies the device. (Video capture devices only.)
		//"WaveInID": The identifier for an audio capture device. (Audio capture devices only.)
		//Filter Info================
		printf("%s\n",W2A(varName.bstrVal));
		VariantClear(&varName);
		//========================
		IBaseFilter *pFilter;
		hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,(void**)&pFilter);
		if (!pFilter){
			continue;
		}
		IEnumPins * pinEnum = NULL;
		if (FAILED(pFilter->EnumPins(&pinEnum))){
			pinEnum->Release();
			continue;	
		}
		pinEnum->Reset();
		IPin * pin = NULL;
		ULONG fetchCount = 0;
		//Pin Info
		while (SUCCEEDED(pinEnum->Next(1, &pin, &fetchCount)) && fetchCount){
			if (!pin){
				continue;
			}
			PIN_INFO PinInfo;
			if (FAILED(pin->QueryPinInfo(&PinInfo))){
				continue;
			}
			printf("\tPin:");
				switch(PinInfo.dir){
				case PINDIR_INPUT:printf("Input\t");break;
				case PINDIR_OUTPUT:printf("Output\t");break;
				default:printf("Unknown\n");break;
			}
			printf("%s\n",W2A(PinInfo.achName));
					
			//MediaType
			/*
			IEnumMediaTypes **ppEnum=NULL;
			AM_MEDIA_TYPE   **ppType=NULL;
			if( FAILED( pin->EnumMediaTypes( ppEnum )) )
				break;
			(*ppEnum)->Reset();

			ULONG uFetched = 0;
			if( FAILED( (*ppEnum)->Next( 1, ppType, &uFetched )) )
				break;
			printf("\t\tMediaType:");
			for(int i=0;i<uFetched;i++){
				//Video
				char *MEDIATYPE_Video_str=GuidToString(MEDIATYPE_Video);
				//Video Subtype Example
				//Bitstream:
				//MEDIASUBTYPE_H264
				//Uncompressed
				//MEDIASUBTYPE_YUY2
				//MEDIASUBTYPE_UYVY
				//Audio
				char *MEDIATYPE_Audio_str=GuidToString(MEDIATYPE_Audio);
				//Audio Subtype Example
				//Bitstream:
				//MEDIASUBTYPE_PCM
				//Uncompressed
				//MEDIASUBTYPE_MPEG_HEAAC
				//MEDIASUBTYPE_DOLBY_AC3
				char *majortype_str=GuidToString(ppType[i]->majortype);
								
				if(strcmp(majortype_str,MEDIATYPE_Video_str)==0){
					printf("Video\n");
				}else if(strcmp(majortype_str,MEDIATYPE_Audio_str)==0){
					printf("Audio\n");
				}else{
					printf("Other\n");
				}
			}
			*/
			//=================
			pin->Release();
				
		}
		pinEnum->Release();
		
		//
		pPropBag->Release();
		pMoniker->Release();
	}
	pEnumCat->Release();
	pSysDevEnum->Release();
	printf("==================================\n");
	CoUninitialize();
}

