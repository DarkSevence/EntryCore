#include "StdAfx.h"
#include "MsApplication.h"

CMSApplication::CMSApplication()
{
}

CMSApplication::~CMSApplication()
{
//	for (TWindowClassSet::iterator i=ms_stWCSet.begin(); i!=ms_stWCSet.end(); ++i)
//		UnregisterClass(*i, ms_hInstance);
}

bool CMSApplication::Initialize(HINSTANCE hInstance) {
	ms_hInstance = hInstance;

	// Zak³adam, ¿e brak wartoœci dla ms_hInstance to b³¹d.
	// Je¿eli inicjalizacja siê powiedzie, zwróæ true.
	return (ms_hInstance != nullptr);
}

void CMSApplication::MessageLoop()
{
	while (MessageProcess());
}

bool CMSApplication::IsMessage()
{
	MSG msg;

	if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		return false;

	return true;
}

bool CMSApplication::MessageProcess()
{
	MSG msg;

	if (!GetMessage(&msg, NULL, 0, 0))
		return false;

	TranslateMessage(&msg);
	DispatchMessage(&msg);
	return true;
}

LRESULT CMSApplication::WindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg)
	{
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
	}

	return CMSWindow::WindowProcedure(hWnd, uiMsg, wParam, lParam);
}