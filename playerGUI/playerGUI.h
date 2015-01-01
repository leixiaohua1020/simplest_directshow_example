/**
 * 最简单的基于DirectShow的播放器-图形界面版
 * Simplest DirectShow Player GUI
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序是一个最简单的基于DirectShow的图形界面播放器。
 * 适合初学者学习DirectShow。
 *
 * This example is the simplest Player based on DirectShow.
 * And it has a simple Graphical User Interface.
 * Suitable for the beginner of DirectShow.
 */

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// CplayerGUIApp:
// 有关此类的实现，请参阅 playerGUI.cpp
//

class CplayerGUIApp : public CWinApp
{
public:
	CplayerGUIApp();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CplayerGUIApp theApp;