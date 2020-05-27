#include "pch.h"

//-----------------------------------
// bmp 이미지 크기
//-----------------------------------
#define BMP_WIDTH 1920
#define BMP_HEIGHT 1080

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
void BitMap();

//--------------------------------
// 전역 변수 설정
//--------------------------------
LPCWSTR lpszClass = L"OpenBitMap";
HINSTANCE g_hInstance;
HDC hdc;


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrecInstance,
    LPSTR lpszCmdParam, int nCmdShow)
{
    HWND hWnd;
    MSG Message;
    WNDCLASS wndClass;

    g_hInstance = hInstance;

    // 클래스 설정
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wndClass.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    wndClass.hInstance = g_hInstance;
    wndClass.lpfnWndProc = (WNDPROC)WndProc;
    wndClass.lpszClassName = lpszClass;
    wndClass.lpszMenuName = NULL;
    wndClass.style = CS_HREDRAW | CS_VREDRAW;

    // 윈도우 클래스 등록
    RegisterClassW(&wndClass);

    // CreateWindow
    hWnd = CreateWindowW
    (
        lpszClass,
        L"Open Bit Map",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        BMP_WIDTH,
        BMP_HEIGHT,
        NULL,
        (HMENU)NULL,
        g_hInstance,
        NULL
    );

    if (!hWnd)
    {
        DWORD error = GetLastError();
        return FALSE;
    }

    // 만들어진 윈도우를 화면에 출력
    ShowWindow(hWnd, nCmdShow);

    // 메시지 루프
    while (GetMessageW(&Message, NULL, 0, 0))
    {
        TranslateMessage(&Message);
        DispatchMessageW(&Message);
    }

    return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;

    switch (iMessage)
    {
    case WM_CREATE:
        // 클라이언트 영역 셋팅
        InvalidateRect(hWnd, NULL, FALSE);
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        // hdc 사용하는 그리기 코드
        BitMap();

        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hWnd, iMessage, wParam, lParam);
    }

    return 0;
}

void BitMap()
{
    // 파일 선언
    FILE* pBmpFile;

    // bmp 파일 개방
    _wfopen_s(&pBmpFile, L"sample2.bmp", L"rb");
    if (pBmpFile == nullptr)
    {
        MessageBoxW(NULL, L"sample2.bmp Open Error..", NULL, MB_OK);
        fclose(pBmpFile);
        return;
    }

    // 헤더 구조체 선언
    BITMAPFILEHEADER bitmapFileHeader;
    BITMAPINFOHEADER bitmapInfoHeader;

    // 헤더 크기
    DWORD headerSize = sizeof(BITMAPFILEHEADER);
    DWORD infoSize = sizeof(BITMAPINFOHEADER);

    // bmp 헤더 정보 읽기
    fread(&bitmapFileHeader, headerSize, 1, pBmpFile);
    fread(&bitmapInfoHeader, infoSize, 1, pBmpFile);

    // 한 줄당 실제 픽셀 갯수
    LONG lWidth = bitmapInfoHeader.biWidth * (bitmapInfoHeader.biBitCount / 8);

    //--------------------------------------------------------------------------------
    // pitch 계산법
    // 32bit -> 4byte 정렬
    // 64bit -> 생각 좀 해봐야 한다
    // 3(이진수 011) 올려서 4byte 정렬을 해준 다음 올려준 3(이진수 011)를 날림
    //--------------------------------------------------------------------------------
    int iPitch = (lWidth + 3) & ~3;

    // 이미지 크기
    int iImageSize = iPitch * bitmapInfoHeader.biHeight;

    //---------------------------------------------------------
    // DIB(BMP)는 이미지가 뒤집혀져 있다
    // 뒤집혀진 이미지가 정상 DIB구조 이므로, 
    // 뒤집혀진 채로 읽어서 DIB 출력 해주면 그림이 출력된다
    //---------------------------------------------------------

    // 이미지 생성
    BYTE* pImage = new BYTE[iImageSize];

    fseek(pBmpFile, bitmapFileHeader.bfOffBits, SEEK_SET);

    // bmp 파일 읽기
    fread(pImage, iImageSize, 1, pBmpFile);

    // 파일 닫기
    fclose(pBmpFile);

    //// 이미지 변조
    //BYTE* pTemp = pImage;
    //BYTE* pImageOld = pImage;

    // DIB -> DC 출력 함수
    StretchDIBits
    (
        hdc,                                    // 목적지 DC
        0,                                      // x 좌표
        0,                                      // y 좌표
        iPitch,                                 // 목적지 너비
        bitmapInfoHeader.biHeight,              // 목적지 높이
        0,                                      // 출력소스 x 좌표
        0,                                      // 출력소스 y 좌표
        iPitch,                                 // 출력소스 너비
        bitmapInfoHeader.biHeight,              // 출력소스 높이
        pImage,                                 // 이미지가 위치한 포인터
        (LPBITMAPINFO)(&bitmapInfoHeader),      // BITMAPINFO = (BITMAPINFOHEADER + RGBQUAD) -> type캐스팅 해야한다. 
        DIB_RGB_COLORS,                         // DIB_RGB_COLORS
        SRCCOPY                                 // 출력모드
    );

    // 동적 할당 해제
    delete[] pImage;

    return;
}

