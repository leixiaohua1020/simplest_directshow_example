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

#define OUTPUT_PIN       1
#define OUTPUT_MEDIATYPE 1


char* GuidToString(const GUID &guid)
{
	int buf_len=64;
	char *buf =(char *)malloc(buf_len);
	_snprintf(
		buf,
		buf_len,
		"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
	//printf("%s\n",buf);
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
	printf("===============Directshow Filters ===============\n");
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr)){
		return hr;
	}

	IEnumMoniker *pEnumCat = NULL;
	//Category	
	/************************************************************************
	Friendly Name	                      CLSID
	-------------------------------------------------------------------------
	Audio Capture Sources	              CLSID_AudioInputDeviceCategory
	Audio Compressors	                  CLSID_AudioCompressorCategory
	Audio Renderers	                      CLSID_AudioRendererCategory
	Device Control Filters	              CLSID_DeviceControlCategory
	DirectShow Filters	                  CLSID_LegacyAmFilterCategory
	External Renderers	                  CLSID_TransmitCategory
	Midi Renderers	                      CLSID_MidiRendererCategory
	Video Capture Sources	              CLSID_VideoInputDeviceCategory
	Video Compressors	                  CLSID_VideoCompressorCategory
	WDM Stream Decompression Devices	  CLSID_DVDHWDecodersCategory
	WDM Streaming Capture Devices	      AM_KSCATEGORY_CAPTURE
	WDM Streaming Crossbar Devices	      AM_KSCATEGORY_CROSSBAR
	WDM Streaming Rendering Devices	      AM_KSCATEGORY_RENDER
	WDM Streaming Tee/Splitter Devices	  AM_KSCATEGORY_SPLITTER
	WDM Streaming TV Audio Devices	      AM_KSCATEGORY_TVAUDIO
	WDM Streaming TV Tuner Devices	      AM_KSCATEGORY_TVTUNER
	WDM Streaming VBI Codecs	          AM_KSCATEGORY_VBICODEC 
	************************************************************************/
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoCompressorCategory, &pEnumCat, 0);
	//hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
	//hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioCompressorCategory, &pEnumCat, 0);
	//hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEnumCat, 0);
	//hr = pSysDevEnum->CreateClassEnumerator(CLSID_MediaMultiplexerCategory, &pEnumCat, 0);
	//hr = pSysDevEnum->CreateClassEnumerator(CLSID_LegacyAmFilterCategory, &pEnumCat, 0);

	if (hr != S_OK) {
		pSysDevEnum->Release();
		return -1;
	}
	
	IMoniker *pMoniker = NULL;
	ULONG monikerFetched;
	//Filter
	while(pEnumCat->Next(1, &pMoniker, &monikerFetched) == S_OK){
		IPropertyBag *pPropBag;
		VARIANT varName;
		IBaseFilter *pFilter;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,(void **)&pPropBag);
		if (FAILED(hr)){
			pMoniker->Release();
			continue;
		}
		VariantInit(&varName);
		hr = pPropBag->Read(L"FriendlyName", &varName, 0);
		//"FriendlyName": The name of the device.
		//"Description": A description of the device.
		//Filter Info================
		printf("[%s]\n",W2A(varName.bstrVal));
		VariantClear(&varName);
		//========================
#if OUTPUT_PIN
		hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,(void**)&pFilter);
		if (!pFilter){
			continue;
		}

		IEnumPins * pinEnum = NULL;
		IPin * pin = NULL;
		ULONG pinFetched = 0;
		if (FAILED(pFilter->EnumPins(&pinEnum))){
			pinEnum->Release();
			continue;	
		}
		pinEnum->Reset();
		//Pin Info
		while (SUCCEEDED(pinEnum->Next(1, &pin, &pinFetched)) && pinFetched){
			if (!pin){
				continue;
			}
			PIN_INFO pinInfo;
			if (FAILED(pin->QueryPinInfo(&pinInfo))){
				continue;
			}
			printf("\t[Pin] ");
				switch(pinInfo.dir){
				case PINDIR_INPUT:printf("Dir:Input  \t");break;
				case PINDIR_OUTPUT:printf("Dir:Output \t");break;
				default:printf("Dir:Unknown\n");break;
			}
			printf("Name:%s\n",W2A(pinInfo.achName));
					
			//MediaType
#if OUTPUT_MEDIATYPE
			IEnumMediaTypes *mtEnum=NULL;
			AM_MEDIA_TYPE   *mt=NULL;
			if( FAILED( pin->EnumMediaTypes( &mtEnum )) )
				break;
			mtEnum->Reset();
			
			ULONG mtFetched = 0;

			while (SUCCEEDED(mtEnum->Next(1, &mt, &mtFetched)) && mtFetched){

				printf("\t\t[MediaType]\n");
				//Video
				char *MEDIATYPE_Video_str=GuidToString(MEDIATYPE_Video);
				//Audio
				char *MEDIATYPE_Audio_str=GuidToString(MEDIATYPE_Audio);
				//Stream
				char *MEDIATYPE_Stream_str=GuidToString(MEDIATYPE_Stream);
				//Majortype
				char *majortype_str=GuidToString(mt->majortype);
				//Subtype
				char *subtype_str=GuidToString(mt->subtype);

				printf("\t\t  Majortype:");
				if(strcmp(majortype_str,MEDIATYPE_Video_str)==0){
					printf("Video\n");
				}else if(strcmp(majortype_str,MEDIATYPE_Audio_str)==0){
					printf("Audio\n");
				}else if(strcmp(majortype_str,MEDIATYPE_Stream_str)==0){
					printf("Stream\n");
				}else{
					printf("Other\n");
				}
				printf("\t\t  Subtype GUID:%s",subtype_str);

				free(MEDIATYPE_Video_str);
				free(MEDIATYPE_Audio_str);
				free(MEDIATYPE_Stream_str);
				free(subtype_str);
				free(majortype_str);
				printf("\n");

			}
#endif
			pin->Release();
				
		}
		pinEnum->Release();

		pFilter->Release();
#endif
		
		pPropBag->Release();
		pMoniker->Release();
	}
	pEnumCat->Release();
	pSysDevEnum->Release();
	printf("=================================================\n");
	CoUninitialize();
}

