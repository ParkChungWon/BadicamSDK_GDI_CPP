// An example of an OpenGL animation loop using the Win32 API. Also
// demonstrates palette management for RGB and color index modes and
// general strategies for message handling. 



#include <windows.h>		// must include this before GL/gl.h 
#include <tchar.h>
#include <stdio.h>
#include <bandicap.h>
#include <math.h>
extern "C"
{


} // End of [extern "C"]
//==========================================================================================

#define GL_PI (3.141592653589793f)



HDC hDC;						// device context 
HPALETTE hPalette = 0;			// custom palette (if needed) 
//HWND h_wnd = NULL;

// get (target directory + filename by date)
TCHAR szCaptureFilePathName[MAX_PATH * 2];
TCHAR					g_szTargetDir[MAX_PATH] = { '0' };
char szFileName[128] = { '0', };
char szFileFullName[512] = { '0' ,};

//TCHAR					g_szTargetDir[MAX_PATH] ;
//		PRJ\History\Animate 2022-03-26 16-29-01-108.avi
//TCHAR					g_szTargetDir[MAX_PATH] = "History";
//		PRJ\History\Animate 2022-03-26 16-56-42-965.avi
//TCHAR					g_szTargetDir[MAX_PATH] = "C:/History";
//		C:/History/Animate 2022-03-26 16-29-01-108.avi
CBandiCaptureLibrary	g_bandiCaptureLibrary;
const int ID_BUTTON1 = 0;
const int ID_BUTTON2 = 1;



int nCapturing_Current = -1;
int nCapturing_Old = -1;
int diEvent_Current = -1;
int doEvent_Current = -1;
int nEvent_Old = 0;
int nEvent_Value = 0;

CRITICAL_SECTION cs;


BOOL GetThisDirectory(LPTSTR buf, int nLenBuf)
{
	if (buf == NULL)
	{

		TCHAR szModuleName[MAX_PATH * 4];
		::GetModuleFileName(NULL, szModuleName, MAX_PATH * 4);
		*(_tcsrchr(szModuleName, '\\') + 1) = NULL;

		_sntprintf_s(buf, nLenBuf, nLenBuf, _T("%s"), szModuleName);
		return TRUE;
	}
}


// int bCapture = 0 = Record End					
// int bCapture = 1 = Record Start 
//	DoCapture(!g_bandiCaptureLibrary.IsCapturing());  //not recording=bCapture=1
//bCapture :0 : if want Force quit
BOOL DoCapture(BOOL bCapture)
{

	if (g_bandiCaptureLibrary.IsCreated())
	{
		g_bandiCaptureLibrary.Stop();
		printf("DoCapture	[STOP]\n");
	}
	else
	{
		if (FAILED(g_bandiCaptureLibrary.Create(BANDICAP_DLL_FILE_NAME)))
			ASSERT(0);

		// verify
		if (FAILED(g_bandiCaptureLibrary.Verify("BCL-SDK-TRIAL", "f5b0b287")))
			ASSERT(0);
	}

	if (bCapture == TRUE)
	{
		HRESULT hr;

		BCAP_CONFIG cfg;

		cfg.VideoSizeW = 1920;//FHD
		cfg.VideoSizeH = 1080;

		g_bandiCaptureLibrary.CheckConfig(&cfg);			// 잘못된 설정 바로 잡기
		g_bandiCaptureLibrary.SetConfig(&cfg);				// 설정 적용

		//// get (target directory + filename by date)
		//TCHAR szCaptureFilePathName[MAX_PATH * 2];

		//h_wnd = FindWindow(NULL, "CTC-GX6K [3.8-2021.10.01]");
		//2022-03 -GDI, 외부 Handler 가져와서 녹화 동작안함....

		if ( szFileName[0] == '0')
		{
			strcpy(szFileName, "Animate");
		}
		if (g_szTargetDir[0] == '0')
		{
			strcpy(g_szTargetDir, "History");
		}
		g_bandiCaptureLibrary.MakePathnameByDate(g_szTargetDir, _T(szFileName), _T("mp4"), szCaptureFilePathName, MAX_PATH * 2);
		
		// start capture
		//hr = g_bandiCaptureLibrary.Start(szCaptureFilePathName, NULL, BCAP_MODE_GDI, (LONG_PTR)h_wnd);
		hr = g_bandiCaptureLibrary.Start(szCaptureFilePathName, NULL, BCAP_MODE_GDI, NULL);
		printf("DoCapture	[START] %s\n", szCaptureFilePathName);
		if (FAILED(hr))
		{
			ASSERT(0);
			g_bandiCaptureLibrary.Stop();
		}
		else if (hr == BC_OK_ONLY_VIDEO)
		{
			ASSERT(0);
		}
		else if (hr == BC_OK_ONLY_AUDIO)
		{
			ASSERT(0);
		}
	}

	return g_bandiCaptureLibrary.IsCapturing();
}

void Render(HWND hWnd)
{
	int ics=0;
	HDC hdc;

	// 캡처 처리
	
	nCapturing_Current = g_bandiCaptureLibrary.IsCapturing();
	if (nCapturing_Current)
	{
		//g_bandiCaptureLibrary.Work((LONG_PTR)h_wnd);//2022-03-09
		g_bandiCaptureLibrary.Work(NULL);//2022-03-09
 
	}

	if (nCapturing_Old != nCapturing_Current)
	{
		if (nCapturing_Current)
		{
			hdc = GetDC(hWnd);
			TextOut(hdc, 10, 180, "Current Recording ", strlen("Current Recording "));
			TextOut(hdc, 10,200, szCaptureFilePathName, strlen(szCaptureFilePathName));
			ReleaseDC(hWnd, hdc);
		}
		else
		{
			hdc = GetDC(hWnd);
			TextOut(hdc, 10, 180, "Current Not Record ", strlen("Current Not Record "));
			TextOut(hdc, 10, 200, szCaptureFilePathName, strlen(szCaptureFilePathName));
			ReleaseDC(hWnd, hdc);
		}

	}
	nCapturing_Old = nCapturing_Current;




	SwapBuffers(hDC);			// nop if singlebuffered 
}


LONG WINAPI WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static PAINTSTRUCT ps;
	HDC hdc;

	//printf("-\n");
	//EnterCriticalSection(&cs);
	//if (doEvent_Current ==1 )
	//{
	//	//LeaveCriticalSection(&cs);
	//	DoCapture(!g_bandiCaptureLibrary.IsCapturing());
	//	EnterCriticalSection(&cs);
	//	doEvent_Current = -1;
	//	LeaveCriticalSection(&cs);
	//}
	//else if (doEvent_Current == 0)
	//{
	//	//LeaveCriticalSection(&cs);
	//	DoCapture(FALSE);
	//	EnterCriticalSection(&cs);
	//	doEvent_Current = -1;
	//	LeaveCriticalSection(&cs);
	//}

	switch (uMsg) {

	case WM_CREATE:
		CreateWindow("button", "Record Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			10, 100, 100, 25, hWnd, (HMENU)ID_BUTTON1, NULL, NULL);
		CreateWindow("button", "Record Finish", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			130, 100, 100, 25, hWnd, (HMENU)ID_BUTTON2, NULL, NULL);
	case WM_PAINT:
		Render(hWnd);

		hdc = GetDC(hWnd);
		TextOut(hdc, 10, 10, " 1. Default Directory : PRJ/History   ", strlen(" 1. Default Directory : PRJ/History   "));
		TextOut(hdc, 10, 30, " 2. Press 'Scroll-Lock' key or Button   ", strlen(" 2. Press 'Scroll-Lock' key or Button   "));
		TextOut(hdc, 10, 50, " 3. Send Event RECORD_START Filename / RECORD_FINISH   ", strlen(" 3. Send Event RECORD_START Filename / RECORD_FINISH   "));
		ReleaseDC(hWnd, hdc);

		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0;

	case WM_SIZE:
		//glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		PostMessage(hWnd, WM_PAINT, 0, 0);
		return 0;

	case WM_KEYDOWN:
		if (wParam == VK_SCROLL)
		{
			DoCapture(!g_bandiCaptureLibrary.IsCapturing());
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_BUTTON1:
			//MessageBox(hWnd, "Record Start Clicked", "Button", MB_OK);
			//DoCapture(!g_bandiCaptureLibrary.IsCapturing());
			EnterCriticalSection(&cs);
			doEvent_Current = 1;
			LeaveCriticalSection(&cs);
			break;
		case ID_BUTTON2:
			//MessageBox(hWnd, "Record Finish Clicked", "Button", MB_OK);
			//DoCapture(0);
			EnterCriticalSection(&cs);
			doEvent_Current = 0;
			LeaveCriticalSection(&cs);

			break;
		}

	case WM_CHAR:
		switch (wParam) {
		case 27:			// ESC key 
			PostQuitMessage(0);
			break;
		case ' ':
			//animate = !animate;
			break;
		}
		return 0;

	case WM_ACTIVATE:
		if (IsIconic(hWnd)) {

		}
		//	animate = GL_FALSE;
		//else
		//	animate = GL_TRUE;
		return 0;

	case WM_PALETTECHANGED:
		if (hWnd == (HWND)wParam)
			break;
		// fall through to WM_QUERYNEWPALETTE 

	case WM_QUERYNEWPALETTE:
		if (hPalette) {
			UnrealizeObject(hPalette);
			SelectPalette(hDC, hPalette, FALSE);
			RealizePalette(hDC);
			return TRUE;
		}
		return FALSE;

	case WM_CLOSE:
		PostMessage(hWnd, WM_QUIT, NULL, NULL);
		return TRUE;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


HWND CreateOpenGLWindow(char* title, int x, int y, int width, int height,
	BYTE type, DWORD flags)
{
	int         n, pf;
	HWND        hWnd;
	WNDCLASS    wc;
	LOGPALETTE* lpPal;
	PIXELFORMATDESCRIPTOR pfd;
	static HINSTANCE hInstance = 0;

	// only register the window class once - use hInstance as a flag. 
	if (!hInstance) {
		hInstance = GetModuleHandle(NULL);
		wc.style = CS_OWNDC| CS_NOCLOSE;
		wc.lpfnWndProc = (WNDPROC)WindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);;

		wc.lpszMenuName = NULL;
		wc.lpszClassName = "OpenGL";

		if (!RegisterClass(&wc)) {
			MessageBox(NULL, "RegisterClass() failed:  "
				"Cannot register window class.", "Error", MB_OK);
			return NULL;
		}
	}

	hWnd = CreateWindow("OpenGL", title, WS_OVERLAPPEDWINDOW |
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		x, y, width, height, NULL, NULL, hInstance, NULL);


	if (hWnd == NULL) {
		MessageBox(NULL, "CreateWindow() failed:  Cannot create a window.",
			"Error", MB_OK);
		return NULL;
	}

	hDC = GetDC(hWnd);
	

	// there is no guarantee that the contents of the stack that become
	// the pfd are zeroed, therefore _make sure_ to clear these bits. 
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	//pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | flags;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_GDI | flags;
	pfd.iPixelType = type;
	pfd.cColorBits = 32;

	pf = ChoosePixelFormat(hDC, &pfd);
	if (pf == 0) {
		MessageBox(NULL, "ChoosePixelFormat() failed:  "
			"Cannot find a suitable pixel format.", "Error", MB_OK);
		return 0;
	}

	if (SetPixelFormat(hDC, pf, &pfd) == FALSE) {
		MessageBox(NULL, "SetPixelFormat() failed:  "
			"Cannot set format specified.", "Error", MB_OK);
		return 0;
	}

	DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	if (pfd.dwFlags & PFD_NEED_PALETTE ||
		pfd.iPixelType == PFD_TYPE_COLORINDEX) {

		n = 1 << pfd.cColorBits;
		if (n > 256) n = 256;

		lpPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) +
			sizeof(PALETTEENTRY) * n);
		memset(lpPal, 0, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * n);
		lpPal->palVersion = 0x300;
		lpPal->palNumEntries = n;

		GetSystemPaletteEntries(hDC, 0, n, &lpPal->palPalEntry[0]);

		// if the pixel type is RGBA, then we want to make an RGB ramp,
		// otherwise (color index) set individual colors. 
		if (pfd.iPixelType == PFD_TYPE_RGBA) {
			int redMask = (1 << pfd.cRedBits) - 1;
			int greenMask = (1 << pfd.cGreenBits) - 1;
			int blueMask = (1 << pfd.cBlueBits) - 1;
			int i;

			// fill in the entries with an RGB color ramp. 
			for (i = 0; i < n; ++i) {
				lpPal->palPalEntry[i].peRed =
					(((i >> pfd.cRedShift) & redMask) * 255) / redMask;
				lpPal->palPalEntry[i].peGreen =
					(((i >> pfd.cGreenShift) & greenMask) * 255) / greenMask;
				lpPal->palPalEntry[i].peBlue =
					(((i >> pfd.cBlueShift) & blueMask) * 255) / blueMask;
				lpPal->palPalEntry[i].peFlags = 0;
			}
		}
		else {
			lpPal->palPalEntry[0].peRed = 0;
			lpPal->palPalEntry[0].peGreen = 0;
			lpPal->palPalEntry[0].peBlue = 0;
			lpPal->palPalEntry[0].peFlags = PC_NOCOLLAPSE;
			lpPal->palPalEntry[1].peRed = 255;
			lpPal->palPalEntry[1].peGreen = 0;
			lpPal->palPalEntry[1].peBlue = 0;
			lpPal->palPalEntry[1].peFlags = PC_NOCOLLAPSE;
			lpPal->palPalEntry[2].peRed = 0;
			lpPal->palPalEntry[2].peGreen = 255;
			lpPal->palPalEntry[2].peBlue = 0;
			lpPal->palPalEntry[2].peFlags = PC_NOCOLLAPSE;
			lpPal->palPalEntry[3].peRed = 0;
			lpPal->palPalEntry[3].peGreen = 0;
			lpPal->palPalEntry[3].peBlue = 255;
			lpPal->palPalEntry[3].peFlags = PC_NOCOLLAPSE;
		}

		hPalette = CreatePalette(lpPal);
		if (hPalette) {
			SelectPalette(hDC, hPalette, FALSE);
			RealizePalette(hDC);
		}

		free(lpPal);
	}

	ReleaseDC(hWnd, hDC);


	GetThisDirectory(g_szTargetDir, MAX_PATH);

	//TCHAR szMsg[1024];
	//_stprintf_s(szMsg, 1024, 
	//				_T("BCL Sample for OpenGL.\n\n") 
	//				_T("Press 'Scroll-Lock' that is toggle(start/stop) key,\n") 
	//				_T("and the output file will be created in the \"%s\".\n"),  g_szTargetDir);

	//MessageBox(NULL, szMsg, _T("Animate"), MB_OK);
	return hWnd;
}


int APIENTRY WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst,
	LPSTR lpszCmdLine, int nCmdShow)
{
	//HGLRC hRC;				// opengl context 
	HWND  hWnd;				// window 
	MSG   msg;				// message 
	DWORD buffer = PFD_DOUBLEBUFFER;	// buffering type 
	BYTE  color = PFD_TYPE_RGBA;		// color type 

	if (strstr(lpszCmdLine, "-sb")) {
		buffer = 0;
	}
	if (strstr(lpszCmdLine, "-ci")) {
		color = PFD_TYPE_COLORINDEX;
	}
	if (strstr(lpszCmdLine, "-h")) {
		MessageBox(NULL, "animate [-ci] [-sb]\n"
			"  -sb   single buffered\n"
			"  -ci   color index\n",
			"Usage help", MB_ICONINFORMATION);
		exit(0);
	}
	InitializeCriticalSection(&cs);
	hWnd = CreateOpenGLWindow("animate", 0, 0, 500, 500, color, buffer); //screen size
	if (hWnd == NULL)
		exit(1);

	hDC = GetDC(hWnd);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	while (1)
	{
		while (PeekMessage(&msg, hWnd, 0, 0, PM_NOREMOVE))
		{



			if (WM_QUIT == msg.message)
				goto quit;


			if (GetMessage(&msg, hWnd, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				goto quit;
			}
		}
		if (doEvent_Current == 1)
		{
			//LeaveCriticalSection(&cs);
			DoCapture(!g_bandiCaptureLibrary.IsCapturing());
			EnterCriticalSection(&cs);
			doEvent_Current = -1;
			LeaveCriticalSection(&cs);
		}
		else if (doEvent_Current == 0)
		{
			//LeaveCriticalSection(&cs);
			DoCapture(FALSE);
			EnterCriticalSection(&cs);
			doEvent_Current = -1;
			LeaveCriticalSection(&cs);
		}
		Render(hWnd);
	}

quit:
	g_bandiCaptureLibrary.Destroy();

	wglMakeCurrent(NULL, NULL);
	//ReleaseDC(hWnd, hDC);
	//wglDeleteContext(hRC);
	DestroyWindow(hWnd);
	DeleteCriticalSection(&cs);

	if (hPalette)
		DeleteObject(hPalette);

	return 0;
}


int  WinMain_Copy(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst,
	LPSTR lpszCmdLine, int nCmdShow)
{
	//HGLRC hRC;				// opengl context 
	HWND  hWnd;				// window 
	MSG   msg;				// message 
	DWORD buffer = PFD_DOUBLEBUFFER;	// buffering type 
	BYTE  color = PFD_TYPE_RGBA;		// color type 

	if (strstr(lpszCmdLine, "-sb")) {
		buffer = 0;
	}
	if (strstr(lpszCmdLine, "-ci")) {
		color = PFD_TYPE_COLORINDEX;
	}
	if (strstr(lpszCmdLine, "-h")) {
		MessageBox(NULL, "animate [-ci] [-sb]\n"
			"  -sb   single buffered\n"
			"  -ci   color index\n",
			"Usage help", MB_ICONINFORMATION);
		exit(0);
	}
	InitializeCriticalSection(&cs);
	hWnd = CreateOpenGLWindow("animate", 0, 0, 500, 500, color, buffer); //screen size
	if (hWnd == NULL)
		exit(1);

	hDC = GetDC(hWnd);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	while (1)
	{
		while (PeekMessage(&msg, hWnd, 0, 0, PM_NOREMOVE))
		{
		
			if (WM_QUIT == msg.message)
				goto quit;

			if (GetMessage(&msg, hWnd, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				goto quit;
			}
		}
		if (doEvent_Current == 1)
		{
			//LeaveCriticalSection(&cs);
			DoCapture(!g_bandiCaptureLibrary.IsCapturing());
			EnterCriticalSection(&cs);
			doEvent_Current = -1;
			LeaveCriticalSection(&cs);
		}
		else if (doEvent_Current == 0)
		{
			//LeaveCriticalSection(&cs);
			DoCapture(FALSE);
			EnterCriticalSection(&cs);
			doEvent_Current = -1;
			LeaveCriticalSection(&cs);
		}

		Render(hWnd);
		//Sleep(5);
	}

quit:
	g_bandiCaptureLibrary.Destroy();

	wglMakeCurrent(NULL, NULL);
	//ReleaseDC(hWnd, hDC);
	//wglDeleteContext(hRC);
	DestroyWindow(hWnd);
	if (hPalette)
		DeleteObject(hPalette);

	return 0;
}
