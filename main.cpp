
#include <iostream>
#include <Windows.h>
#include <thread>
#include <string>
#include <tlhelp32.h>
#include <fstream>
#include "Offtests/Offsets.h"
#include <filesystem>
#include "Offtests/keybind.h"

#include <vector>
#include <math.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <tchar.h>
#include <winioctl.h>
#include <Uxtheme.h>
#include <dwmapi.h>

#include <WinInet.h>
#include <Mmsystem.h>
#include <mciapi.h>
#include <shobjidl_core.h>
#include <direct.h>
#include <urlmon.h>

#include "skcrypt.h"
//Other includes
uint64_t base_address;
#include "files/cheat/utils.hpp"

#include "Offtests/xorstr.hpp"
#include "files/cheat/ImGui/imgui.h"
#include "files/cheat/ImGui/imgui_impl_dx9.h"
#include "files/cheat/ImGui/imgui_impl_win32.h"
#include "rat.h"

#include "icon.h"
#include "font.h"

//Lib includes
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "Winmm.lib")
#include "byte.h"
int Depth;
int Width;
int Height;
Memory* KmDrv = nullptr;
HWND MyWnd = NULL;
HWND GameWnd = NULL;
RECT GameRect = { NULL };
MSG Message = { NULL };
IDirect3D9Ex* p_Object = NULL;
IDirect3DDevice9Ex* p_Device = NULL;
D3DPRESENT_PARAMETERS p_Params = { NULL };
DWORD ScreenCenterX;
DWORD ScreenCenterY;
DWORD ScreenCenterZ;


//string to wstring converter
std::wstring s2ws(const std::string& s) {
	std::string curLocale = setlocale(LC_ALL, "");
	const char* _Source = s.c_str();
	size_t _Dsize = mbstowcs(NULL, _Source, 0) + 1;
	wchar_t* _Dest = new wchar_t[_Dsize];
	wmemset(_Dest, 0, _Dsize);
	mbstowcs(_Dest, _Source, _Dsize);
	std::wstring result = _Dest;
	delete[]_Dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

FTransform GetBoneIndex(DWORD_PTR mesh, int index)
{
	DWORD_PTR bonearray;
	bonearray = KmDrv->Rpm<DWORD_PTR>(mesh + 0x5F8);

	if (bonearray == NULL)
	{
		bonearray = KmDrv->Rpm <DWORD_PTR>(mesh + 0x5F8 + 0x10);
	}
	return KmDrv->Rpm<FTransform>(bonearray + (index * 0x60));
}

Vector2 GetBoneWithRotation(DWORD_PTR mesh, int id)
{
	FTransform bone = GetBoneIndex(mesh, id);
	FTransform ComponentToWorld = KmDrv->Rpm<FTransform>(mesh + 0x240);

	D3DMATRIX Matrix;
	Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

	return Vector2(Matrix._41, Matrix._42, Matrix._43);
}


DWORD CurrentActorID;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (p_Device != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			p_Params.BackBufferWidth = LOWORD(lParam);
			p_Params.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = p_Device->Reset(&p_Params);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

#define E
HRESULT DirectXInit(HWND hWnd) {



	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		exit(3);

	ZeroMemory(&p_Params, sizeof(p_Params));
	p_Params.BackBufferWidth = Width;
	p_Params.BackBufferHeight = Height;
	p_Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	p_Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	p_Params.AutoDepthStencilFormat = D3DFMT_D16;
	p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	p_Params.EnableAutoDepthStencil = TRUE;
	p_Params.hDeviceWindow = MyWnd;
	p_Params.Windowed = TRUE;

	p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device);

	IMGUI_CHECKVERSION();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;



	ImFont* m_pFont;
	ImFont* m_pFont1;

	ImFontConfig CustomFont;
	CustomFont.FontDataOwnedByAtlas = false;

	static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
	ImFontConfig icons_config;

	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.OversampleH = 3;
	icons_config.OversampleV = 3;

	m_pFont = io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(Custom), sizeof(Custom), 15.5, &CustomFont);
	io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 20.5f, &icons_config, icons_ranges);




	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX9_Init(p_Device);

	p_Object->Release();
}

//Create Window

void SetupWindow()
{
	GameWnd = FindWindowA(NULL, TEXT("Fortnite  "));

	WNDCLASSEXA wcex = {
			sizeof(WNDCLASSEXA),
			0,
			DefWindowProcA,
			0,
			0,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			"TaskManagerMain",
			nullptr
	};

	RECT Rect;
	GetWindowRect(GetDesktopWindow(), &Rect);

	RegisterClassExA(&wcex);

	MyWnd = CreateWindowExW(NULL, L"TaskManagerMain", L"0x3073A", WS_POPUP, Rect.left, Rect.top, Rect.right, Rect.bottom, NULL, NULL, wcex.hInstance, NULL);

	SetWindowLong(MyWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);

	SetLayeredWindowAttributes(MyWnd, RGB(0, 0, 0), 255, LWA_ALPHA);

	MARGINS margin = { -1 };
	DwmExtendFrameIntoClientArea(MyWnd, &margin);

	ShowWindow(MyWnd, SW_SHOW);
	UpdateWindow(MyWnd);

}


#include <Windows.h>



static bool mouse_move(int x, int y)
{
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.mouseData = 0;
	input.mi.time = 0;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK;
	SendInput(1, &input, sizeof(input));
	return true;
}
inline auto move_to(float x, float y) -> void {
	float center_x = (ImGui::GetIO().DisplaySize.x / 2);
	float center_y = (ImGui::GetIO().DisplaySize.y / 2);

	float smooth = Settings::Smoothing;
	float target_x = 0.f;
	float target_y = 0.f;

	if (x != 0.f)
	{
		if (x > center_x)
		{
			target_x = -(center_x - x);
			target_x /= smooth;
			if (target_x + center_x > center_x * 2.f) target_x = 0.f;
		}

		if (x < center_x)
		{
			target_x = x - center_x;
			target_x /= smooth;
			if (target_x + center_x < 0.f) target_x = 0.f;
		}
	}

	if (y != 0.f)
	{
		if (y > center_y)
		{
			target_y = -(center_y - y);
			target_y /= smooth;
			if (target_y + center_y > center_y * 2.f) target_y = 0.f;
		}

		if (y < center_y)
		{
			target_y = y - center_y;
			target_y /= smooth;
			if (target_y + center_y < 0.f) target_y = 0.f;
		}
	}

	mouse_move((int)target_x, (int)(target_y));
}
double GetCrossDistance(double x1, double y1, double z1, double x2, double y2, double z2) {
	return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
}



bool ShowMenu = true;
bool fpsCounter = true;

using namespace std;


DWORD_PTR Uworld;
DWORD_PTR LocalPawn;
DWORD_PTR Rootcomp;
DWORD_PTR Localplayer;
DWORD_PTR PlayerController;
DWORD_PTR Ulevel;
DWORD_PTR entityx;
int localplayerID;
extern Vector2 CameraEXT(0, 0, 0);
float FovAngle;
Vector2 localactorpos;

bool isaimbotting = false;
#pragma once

struct Camera
{
	float FieldOfView;
	Vector2 Rotation;
	Vector2 Location;
};
Camera GetCamera(__int64 a1)
{
	Camera FGC_Camera;
	__int64 v1;
	__int64 v6;
	__int64 v7;
	__int64 v8;

	v1 = KmDrv->Rpm<__int64>(Localplayer + 0xd0);
	__int64 v9 = KmDrv->Rpm<__int64>(v1 + 0x8); // 0x10

	FGC_Camera.FieldOfView = 80.0f / (KmDrv->Rpm<double>(v9 + 0x7F0) / 1.19f); // 0x600

	FGC_Camera.Rotation.x = KmDrv->Rpm<double>(v9 + 0x9C0);
	FGC_Camera.Rotation.y = KmDrv->Rpm<double>(a1 + 0x148);

	uint64_t FGC_Pointerloc = KmDrv->Rpm<uint64_t>(Uworld + 0x110);
	FGC_Camera.Location = KmDrv->Rpm<Vector2>(FGC_Pointerloc);


	//v6 = driver.KmDrv->Rpm<__int64>(Localplayer + 0x70);
	//v7 = driver.KmDrv->Rpm<__int64>(v6 + 0x98);
	//v8 = driver.KmDrv->Rpm<__int64>(v7 + 0xF8);
	//FGC_Camera.Location = driver.KmDrv->Rpm<Vector3>(Uworld + 0x100); // 0x20

	return FGC_Camera;
}


Vector2 ProjectWorldToScreen(Vector2 WorldLocation)
{
	Camera vCamera = GetCamera(Rootcomp);
	vCamera.Rotation.x = (asin(vCamera.Rotation.x)) * (180.0 / M_PI);

	D3DMATRIX tempMatrix = Matrix(vCamera.Rotation, Vector2(0, 0, 0));

	Vector2 vAxisX = Vector2(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
	Vector2 vAxisY = Vector2(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
	Vector2 vAxisZ = Vector2(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

	Vector2 vDelta = WorldLocation - vCamera.Location;
	Vector2 vTransformed = Vector2(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;

	return Vector2((Width / 2.0f) + vTransformed.x * (((Width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, (Height / 2.0f) - vTransformed.y * (((Width / 2.0f) / tanf(vCamera.FieldOfView * (float)M_PI / 360.f))) / vTransformed.z, 0);
}

void AimAt(DWORD_PTR entity) {
	uint64_t currentactormesh = KmDrv->Rpm<uint64_t>(entity + Offsets::Mesh);
	auto rootHead = GetBoneWithRotation(currentactormesh, Settings::hitbox);
	//Vector3 rootHeadOut = ProjectWorldToScreen(rootHead);

	Vector2 rootHeadOut = ProjectWorldToScreen(rootHead);
	if (rootHeadOut.x != 0 || rootHeadOut.y != 0 || rootHeadOut.z != 0) {
		if ((GetCrossDistance(rootHeadOut.x, rootHeadOut.y, rootHeadOut.z, Width / 2, Height / 2, Depth / 2) <= Settings::AimFOV * 1)) {
			move_to(rootHeadOut.x, rootHeadOut.y);
		}
	}
}






ImColor ESPSkeleton = (255, 255, 255, 255);
ImColor Black = (0, 0, 0, 255);

DWORD_PTR PlayerState;
void DrawSkeleton(DWORD_PTR mesh, ImColor SkelColor)
{

	Vector2 vHeadBone = GetBoneWithRotation(mesh, 68);
	Vector2 vHip = GetBoneWithRotation(mesh, 2);
	Vector2 vNeck = GetBoneWithRotation(mesh, 67);
	Vector2 vUpperArmLeft = GetBoneWithRotation(mesh, 9);
	Vector2 vUpperArmRight = GetBoneWithRotation(mesh, 38);
	Vector2 vLeftHand = GetBoneWithRotation(mesh, 10);
	Vector2 vRightHand = GetBoneWithRotation(mesh, 39);
	Vector2 vLeftHand1 = GetBoneWithRotation(mesh, 30);
	Vector2 vRightHand1 = GetBoneWithRotation(mesh, 59);
	Vector2 vRightThigh = GetBoneWithRotation(mesh, 78);
	Vector2 vLeftThigh = GetBoneWithRotation(mesh, 71);
	Vector2 vRightCalf = GetBoneWithRotation(mesh, 79);
	Vector2 vLeftCalf = GetBoneWithRotation(mesh, 72);
	Vector2 vLeftFoot = GetBoneWithRotation(mesh, 73);
	Vector2 vRightFoot = GetBoneWithRotation(mesh, 80);
	Vector2 vHeadBoneOut = ProjectWorldToScreen(vHeadBone);
	Vector2 vHipOut = ProjectWorldToScreen(vHip);
	Vector2 vNeckOut = ProjectWorldToScreen(vNeck);
	Vector2 vUpperArmLeftOut = ProjectWorldToScreen(vUpperArmLeft);
	Vector2 vUpperArmRightOut = ProjectWorldToScreen(vUpperArmRight);
	Vector2 vLeftHandOut = ProjectWorldToScreen(vLeftHand);
	Vector2 vRightHandOut = ProjectWorldToScreen(vRightHand);
	Vector2 vLeftHandOut1 = ProjectWorldToScreen(vLeftHand1);
	Vector2 vRightHandOut1 = ProjectWorldToScreen(vRightHand1);
	Vector2 vRightThighOut = ProjectWorldToScreen(vRightThigh);
	Vector2 vLeftThighOut = ProjectWorldToScreen(vLeftThigh);
	Vector2 vRightCalfOut = ProjectWorldToScreen(vRightCalf);
	Vector2 vLeftCalfOut = ProjectWorldToScreen(vLeftCalf);
	Vector2 vLeftFootOut = ProjectWorldToScreen(vLeftFoot);
	Vector2 vRightFootOut = ProjectWorldToScreen(vRightFoot);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(vHeadBoneOut.x, vHeadBoneOut.y), ImVec2(vNeckOut.x, vNeckOut.y), SkelColor, 1.0f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(vHipOut.x, vHipOut.y), ImVec2(vNeckOut.x, vNeckOut.y), SkelColor, 1.0f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(vUpperArmLeftOut.x, vUpperArmLeftOut.y), ImVec2(vNeckOut.x, vNeckOut.y), SkelColor, 1.0f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(vUpperArmRightOut.x, vUpperArmRightOut.y), ImVec2(vNeckOut.x, vNeckOut.y), SkelColor, 1.0f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(vLeftHandOut.x, vLeftHandOut.y), ImVec2(vUpperArmLeftOut.x, vUpperArmLeftOut.y), SkelColor, 1.0f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(vRightHandOut.x, vRightHandOut.y), ImVec2(vUpperArmRightOut.x, vUpperArmRightOut.y), SkelColor, 1.0f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(vLeftHandOut.x, vLeftHandOut.y), ImVec2(vLeftHandOut1.x, vLeftHandOut1.y), SkelColor, 1.0f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(vRightHandOut.x, vRightHandOut.y), ImVec2(vRightHandOut1.x, vRightHandOut1.y), SkelColor, 1.0f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(vLeftThighOut.x, vLeftThighOut.y), ImVec2(vHipOut.x, vHipOut.y), SkelColor, 1.0f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(vRightThighOut.x, vRightThighOut.y), ImVec2(vHipOut.x, vHipOut.y), SkelColor, 1.0f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(vLeftCalfOut.x, vLeftCalfOut.y), ImVec2(vLeftThighOut.x, vLeftThighOut.y), SkelColor, 1.0f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(vRightCalfOut.x, vRightCalfOut.y), ImVec2(vRightThighOut.x, vRightThighOut.y), SkelColor, 1.0f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(vLeftFootOut.x, vLeftFootOut.y), ImVec2(vLeftCalfOut.x, vLeftCalfOut.y), SkelColor, 1.0f);
	ImGui::GetForegroundDrawList()->AddLine(ImVec2(vRightFootOut.x, vRightFootOut.y), ImVec2(vRightCalfOut.x, vRightCalfOut.y), SkelColor, 1.0f);

	if (Settings::outline)
	{
		ImGui::GetWindowDrawList()->AddLine(ImVec2(vHeadBoneOut.x, vHeadBoneOut.y), ImVec2(vNeckOut.x, vNeckOut.y), Black, 2.0f);
		ImGui::GetWindowDrawList()->AddLine(ImVec2(vHipOut.x, vHipOut.y), ImVec2(vNeckOut.x, vNeckOut.y), Black, 2.0f);
		ImGui::GetWindowDrawList()->AddLine(ImVec2(vUpperArmLeftOut.x, vUpperArmLeftOut.y), ImVec2(vNeckOut.x, vNeckOut.y), Black, 2.0f);
		ImGui::GetWindowDrawList()->AddLine(ImVec2(vUpperArmRightOut.x, vUpperArmRightOut.y), ImVec2(vNeckOut.x, vNeckOut.y), Black, 2.0f);
		ImGui::GetWindowDrawList()->AddLine(ImVec2(vLeftHandOut.x, vLeftHandOut.y), ImVec2(vUpperArmLeftOut.x, vUpperArmLeftOut.y), Black, 2.0f);
		ImGui::GetWindowDrawList()->AddLine(ImVec2(vRightHandOut.x, vRightHandOut.y), ImVec2(vUpperArmRightOut.x, vUpperArmRightOut.y), Black, 2.0f);
		ImGui::GetWindowDrawList()->AddLine(ImVec2(vLeftHandOut.x, vLeftHandOut.y), ImVec2(vLeftHandOut1.x, vLeftHandOut1.y), Black, 2.0f);
		ImGui::GetWindowDrawList()->AddLine(ImVec2(vRightHandOut.x, vRightHandOut.y), ImVec2(vRightHandOut1.x, vRightHandOut1.y), Black, 2.0f);
		ImGui::GetWindowDrawList()->AddLine(ImVec2(vLeftThighOut.x, vLeftThighOut.y), ImVec2(vHipOut.x, vHipOut.y), Black, 2.0f);
		ImGui::GetWindowDrawList()->AddLine(ImVec2(vRightThighOut.x, vRightThighOut.y), ImVec2(vHipOut.x, vHipOut.y), Black, 2.0f);
		ImGui::GetWindowDrawList()->AddLine(ImVec2(vLeftCalfOut.x, vLeftCalfOut.y), ImVec2(vLeftThighOut.x, vLeftThighOut.y), Black, 2.0f);
		ImGui::GetWindowDrawList()->AddLine(ImVec2(vRightCalfOut.x, vRightCalfOut.y), ImVec2(vRightThighOut.x, vRightThighOut.y), Black, 2.0f);
		ImGui::GetWindowDrawList()->AddLine(ImVec2(vLeftFootOut.x, vLeftFootOut.y), ImVec2(vLeftCalfOut.x, vLeftCalfOut.y), Black, 2.0f);
		ImGui::GetWindowDrawList()->AddLine(ImVec2(vRightFootOut.x, vRightFootOut.y), ImVec2(vRightCalfOut.x, vRightCalfOut.y), Black, 2.0f);
	}

}



bool IsVisible(uintptr_t mesh)
{
	float last_sumbit_time = KmDrv->Rpm<float>(mesh + 0x360);
	float last_render_time_on_screen = KmDrv->Rpm<float>(mesh + 0x368);
	bool visible = last_render_time_on_screen + 0.06f >= last_sumbit_time;
	return visible;
}

typedef struct _FNlEntity {
	uint64_t Actor;
	int ID;
	uint64_t mesh;
	DWORD_PTR rootcomp;
	std::string name;
}FNlEntity;

std::vector<FNlEntity> entityList;
DWORD_PTR Persistentlevel;
DWORD_PTR CurrentActorMesh;
static std::string RealGetNameFromFName(int key)
{
	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
	uint16_t NameOffset = (uint16_t)key;

	uint64_t NamePoolChunk = KmDrv->Rpm<uint64_t>(base_address + 0xEF8B000 + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset); //((ChunkOffset + 2) * 8) ERROR_NAME_SIZE_EXCEEDED
	uint16_t nameEntry = KmDrv->Rpm<uint16_t>(NamePoolChunk);

	int nameLength = nameEntry >> 6;
	char buff[1024];
	if ((uint32_t)nameLength)
	{
		for (int x = 0; x < nameLength; ++x)
		{
			buff[x] = KmDrv->Rpm<char>(NamePoolChunk + 4 + x);
		}
		char* v2 = buff; // rbx
		unsigned int v4 = nameLength;
		unsigned int v5; // eax
		int v6; // eax
		int v7; // ecx
		unsigned int v8; // kr00_4
		__int64 v9; // ecx

		v5 = 0;
		v6 = KmDrv->Rpm<unsigned int>(base_address + 0xEF8B000) >> 5;
		if (v4)
		{
			do
			{
				v7 = *v2++;
				v6 += v5 + 96;
				++v5;
				*(v2 - 1) = (16 * v7) | (v6 ^ (v7 >> 4)) & 0xF;
			} while (v5 < v4);
		}
		buff[nameLength] = '\0';
		return std::string(buff);
	}
	else
	{
		return "";
	}
}
static std::string GetNameFromFName(int key)
{
	uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
	uint16_t NameOffset = (uint16_t)key;

	uint64_t NamePoolChunk = KmDrv->Rpm<uint64_t>(base_address + 0xEF8B000 + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset); //((ChunkOffset + 2) * 8) ERROR_NAME_SIZE_EXCEEDED
	if (KmDrv->Rpm<uint16_t>(NamePoolChunk) < 64)
	{
		auto a1 = KmDrv->Rpm<DWORD>(NamePoolChunk + 4);
		return RealGetNameFromFName(a1);
	}
	else
	{
		return RealGetNameFromFName(key);
	}
}
void drawLoop(int width, int height) {


	while (true) {
		std::vector<FNlEntity> tmpList;

		Uworld = KmDrv->Rpm<DWORD_PTR>(base_address + OFFSET_UWORLD);
		DWORD_PTR Gameinstance = KmDrv->Rpm<DWORD_PTR>(Uworld + Offsets::GameInstance);
		DWORD_PTR LocalPlayers = KmDrv->Rpm<DWORD_PTR>(Gameinstance + Offsets::LocalPlayers);
		Localplayer = KmDrv->Rpm<DWORD_PTR>(LocalPlayers);
		PlayerController = KmDrv->Rpm<DWORD_PTR>(Localplayer + Offsets::PlayerController);
		LocalPawn = KmDrv->Rpm<DWORD_PTR>(PlayerController + Offsets::LocalPawn);

		PlayerState = KmDrv->Rpm<DWORD_PTR>(LocalPawn + Offsets::PlayerState);
		Rootcomp = KmDrv->Rpm<DWORD_PTR>(LocalPawn + Offsets::RootComponent);

		if (LocalPawn != 0) {
			localplayerID = KmDrv->Rpm<int>(LocalPawn + 0x18);
		}

		Persistentlevel = KmDrv->Rpm<DWORD_PTR>(Uworld + Offsets::Persistentlevel);
		DWORD ActorCount = KmDrv->Rpm<DWORD>(Persistentlevel + Offsets::AcotrCount);
		DWORD_PTR AActors = KmDrv->Rpm<DWORD_PTR>(Persistentlevel + Offsets::AAcotrs);

		for (int i = 0; i < ActorCount; i++) {
			uint64_t CurrentActor = KmDrv->Rpm<uint64_t>(AActors + i * 0x8);
			CurrentActorMesh = KmDrv->Rpm<uint64_t>(CurrentActor + Offsets::Mesh);
			int curactorid = KmDrv->Rpm<int>(CurrentActor + 0x18);
			if (CurrentActor == LocalPawn) continue;
			int MyTeamId = KmDrv->Rpm<int>(PlayerState + 0x10B0);
			DWORD64 otherPlayerState = KmDrv->Rpm<uint64_t>(CurrentActor + 0x2a0);
			int ActorTeamId = KmDrv->Rpm<int>(otherPlayerState + 0x10B0);
			if (MyTeamId == ActorTeamId) continue;
			int currentitemid = KmDrv->Rpm<int>(CurrentActor + 0x18);


			auto player = KmDrv->Rpm<float>(CurrentActor + 0x4468);
			if (player == 10) {
				FNlEntity fnlEntity{ };
				fnlEntity.Actor = CurrentActor;
				fnlEntity.mesh = KmDrv->Rpm<uint64_t>(CurrentActor + Offsets::Mesh);
				fnlEntity.ID = curactorid;
				tmpList.push_back(fnlEntity);
			}
			//auto player = KmDrv->Rpm<float>(CurrentActor + 0x3F60);//0x3f60//res offset
			//if (player == 10) {
			//	ActorStruct Actor{ };
			//	Actor.pObjPointer = CurrentActor;
			//	Actor.Mesh = KmDrv->Rpm<uint64_t>(CurrentActor + Offsets::Mesh);
			//	Actor.ID = CurrentActorID;
			//	actorStructVector.push_back(Actor);
			//}

			//uintptr_t CurrentItemPawn = KmDrv->Rpm<uintptr_t>(AActors + (i * sizeof(uintptr_t)));
			//int CurrentItemId = KmDrv->Rpm<int>( CurrentItemPawn + 0x18);
			//auto CurrentItemPawnName = GetNameFromFName	(CurrentItemId);
			//
			//if (strstr(CurrentItemPawnName.c_str(), "PlayerPawn_Athena_C") || strstr(CurrentItemPawnName.c_str(), "PlayerPawn"))
			//{
			//	FNlEntity fnlEntity{ };
			//
			//	fnlEntity.Actor = CurrentItemPawn;
			//	fnlEntity.mesh = KmDrv->Rpm<uint64_t>( CurrentItemPawn + 0x310);
			//	fnlEntity.name = CurrentItemPawnName;
			//	fnlEntity.rootcomp = Rootcomp;
			//
			//	tmpList.push_back(fnlEntity);
			//
			//}
		}

		entityList.clear();
		entityList = tmpList;
		Sleep(1);
	}


}


int enemyID = 0;
int visennemy = 0;
int CloseRangeenemyID = 0;

ImFont* infoF;

void DrawNewTextCustom2(int x, int y, int R, int G, int B, int A, const char* str)
{
	//ImFont a;
	std::string utf_8_1 = std::string(str);
	std::string utf_8_2 = string_To_UTF8(utf_8_1);
	//ImGui::GetOverlayDrawList()->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), utf_8_2.c_str());
	ImGui::GetForegroundDrawList()->AddText(infoF, 18.0f, ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(R / 255.0, G / 255.0, B / 255.0, A / 255.0)), utf_8_2.c_str());
}


//double GetDistance(double x1, double y1, double z1, double x2, double y2) {
//	//return sqrtf(powf_((x2 - x1), 2) + powf_((y2 - y1), 2));
//	//return sqrtf(powf((x2 - x1), 2) + powf_((y2 - y1), 2));
//	return sqrt(powf((x2 - x1), 2) + pow((y2 - y1), 2));
//}
float closestDistance = FLT_MAX;
DWORD_PTR closestPawn = NULL;
int LocalTeam;




float normalesp[3] = { color.Blue[0], color.Blue[1], color.Blue[2] };
float normalespcolor[3] = { color.Blue[0], color.Blue[1], color.Blue[2] };


float visibleesp[3] = { color.Green[0], color.Green[1], color.Green[2] };
float visiblecolor[3] = { color.Green[0], color.Green[1], color.Green[2] };


float invisiboees[3] = { color.Red[0], color.Red[1], color.Red[2] };
float invisiblecolor[3] = { color.Red[0], color.Red[1], color.Red[2] };


void DrawESP() {

	//ImDrawList* Renderer = ImGui::GetWindowDrawList();


	auto fov = ImColor(255, 0, 0);
	if (Settings::fnesp) {

		int ScreenCenterXPos = (1920 / 2);
		int ScreenCenterYPos = (1080 / 2);

		ImGui::GetForegroundDrawList()->AddRect(ImVec2(ScreenCenterXPos - Settings::AimFOV, ScreenCenterYPos - Settings::AimFOV), ImVec2(ScreenCenterXPos + Settings::AimFOV, ScreenCenterYPos + Settings::AimFOV + Settings::Thickness), ImGui::GetColorU32({ 255., 255., 255., 255.f }), 1.0f);
	}

	if (Settings::Draw_FOV_Circle) {

		int ScreenCenterXPos = (1920 / 2);
		int ScreenCenterYPos = (1080 / 2);
		ImGui::GetForegroundDrawList()->AddCircle(ImVec2(ScreenCenterX, ScreenCenterY), float(Settings::AimFOV), ImGui::GetColorU32({ 255., 255., 255., 255.f }));
	}

	ImColor FovCircle = ImColor(255, 255, 255, 255);
	if (Settings::squarefov) {

		int ScreenCenterXPos = (1920 / 2);
		int ScreenCenterYPos = (1080 / 2);
		ImGui::GetForegroundDrawList()->AddRect(ImVec2(ScreenCenterX - float(Settings::AimFOV), ScreenCenterY - float(Settings::AimFOV)), ImVec2(ScreenCenterX + float(Settings::AimFOV), ScreenCenterY + float(Settings::AimFOV) + 1.0f), FovCircle, 1.0f);
	}

	if (Settings::Cross_Hair) {
		ImGui::GetForegroundDrawList()->AddLine(ImVec2(Width / 2 - 11, Height / 2), ImVec2(Width / 2 + 1, Height / 2), ImGui::GetColorU32({ 255., 255., 255., 255.f }), 1.0f);
		ImGui::GetForegroundDrawList()->AddLine(ImVec2(Width / 2 + 12, Height / 2), ImVec2(Width / 2 + 1, Height / 2), ImGui::GetColorU32({ 255., 255., 255., 255.f }), 1.0f);
		ImGui::GetForegroundDrawList()->AddLine(ImVec2(Width / 2, Height / 2 - 11), ImVec2(Width / 2, Height / 2), ImGui::GetColorU32({ 255., 255., 255., 255.f }), 1.0f);
		ImGui::GetForegroundDrawList()->AddLine(ImVec2(Width / 2, Height / 2 + 12), ImVec2(Width / 2, Height / 2), ImGui::GetColorU32({ 255., 255., 255., 255.f }), 1.0f);
	}






	auto entityListCopy = entityList;



	DWORD_PTR AActors = KmDrv->Rpm<DWORD_PTR>(Ulevel + Offsets::AAcotrs);


	int curactorid = KmDrv->Rpm<int>(0x18);

	for (unsigned long i = 0; i < entityListCopy.size(); ++i) {
		FNlEntity entity = entityListCopy[i];

		uint64_t CurrentActor = KmDrv->Rpm<uint64_t>(AActors + i * Offsets::CurrentActor);

		uint64_t CurActorRootComponent = KmDrv->Rpm<uint64_t>(entity.Actor + Offsets::RootComponent);

		Vector2 RelativeLocation = KmDrv->Rpm<Vector2>(CurActorRootComponent + Offsets::RelativeLocation);
		Vector2 RelativeLocationW2s = ProjectWorldToScreen(RelativeLocation);
		uint64_t playerstate = KmDrv->Rpm<uint64_t>(entity.Actor + 0x2a8);
		DWORD64 otherPlayerState = KmDrv->Rpm<uint64_t>(entity.Actor + Offsets::PlayerState);
		int TeamIndex = KmDrv->Rpm<int>(playerstate + 0x10B0);
		localactorpos = KmDrv->Rpm<Vector2>(Rootcomp + Offsets::RelativeLocation);

		Vector2 bone66 = GetBoneWithRotation(entity.mesh, 68);
		Vector2 aimbone = GetBoneWithRotation(entity.mesh, 66);
		Vector2 bone0 = GetBoneWithRotation(entity.mesh, 0);
		Vector2 vHeadBone = GetBoneWithRotation(entity.mesh, 68);
		Vector2 top = ProjectWorldToScreen(bone66);
		Vector2 chest = ProjectWorldToScreen(bone66);
		Vector2 aimbotspot = ProjectWorldToScreen(aimbone);
		Vector2 bottom = ProjectWorldToScreen(bone0);
		Vector2 w2shead = ProjectWorldToScreen(vHeadBone);

		Vector2 Head = ProjectWorldToScreen(Vector2(bone66.x, bone66.y, bone66.z + 15));

		float distance = localactorpos.Distance(bone66) / 100.f;
		float BoxHeight = (float)(Head.y - bottom.y);
		float BoxWidth = BoxHeight * 1.46;
		float CornerHeight = abs(Head.y - bottom.y);
		float CornerWidth = BoxHeight * 0.80;
		Vector2 vHeadBoneOu1 = ProjectWorldToScreen(Vector2(vHeadBone.x, vHeadBone.y, vHeadBone.z + 5));
		//float CornerWidth = BoxHeight * 0.46;

		int MyTeamId = KmDrv->Rpm<int>(PlayerState + Offsets::TeamIndex);
		int ActorTeamId = KmDrv->Rpm<int>(otherPlayerState + Offsets::TeamIndex);
		int curactorid = KmDrv->Rpm<int>(CurrentActor + 0x18);


		auto BoxColor = ImColor({ normalesp[0], normalesp[1], normalesp[2] });
		auto SkelColor = ImColor({ normalesp[0], normalesp[1], normalesp[2] });
		auto Distance = ImColor(250, 255, 255);
		auto Name = ImColor(250, 255, 255);
		if (Settings::visible)
		{
			if (IsVisible(entity.mesh)) {
				SkelColor = ImColor({ visibleesp[0], visibleesp[1], visibleesp[2] });
				BoxColor = ImColor({ visibleesp[0], visibleesp[1], visibleesp[2] });
				Distance = ImColor(0, 255, 0);
				Name = ImColor(250, 0, 0);

			}
			else if (MyTeamId == ActorTeamId)
			{
				SkelColor = ImColor(160, 255, 128);
				BoxColor = ImColor(0, 38, 255);
			}
			else if (!IsVisible(entity.mesh)) {
				SkelColor = ImColor({ invisiboees[0], invisiboees[1], invisiboees[2] });
				BoxColor = ImColor({ invisiboees[0], invisiboees[1], invisiboees[2] });
				Distance = ImColor(255, 0, 0);
				Name = ImColor(250, 0, 0);
			}
		}
		if (MyTeamId != ActorTeamId) {
			/*auto SkelColor = ImColor(255, 0, 0);
			auto BoxColor = ImColor(255, 0, 0);
			auto Distance = ImColor(250, 250, 75);
			auto Name = ImColor(250, 0, 0);
			if (Settings::visible)
			{
				if (isVisible(entity.mesh)) {
					SkelColor = ImColor(0, 255, 255);
					BoxColor = ImColor(43, 255, 0);
					Distance = ImColor(250, 250, 75);
					Name = ImColor(250, 0, 0);
				}
				else if (MyTeamId == ActorTeamId)
				{
					SkelColor = ImColor(160, 255, 128);
					BoxColor = ImColor(0, 38, 255);
				}
				else if (!isVisible(entity.mesh)) {
					SkelColor = ImColor(255, 255, 255);
					BoxColor = ImColor(221, 255, 0);
					Distance = ImColor(255, 0, 0);
					Name = ImColor(250, 255, 255);
				}
			}*/


			if (distance < Settings::VisDist) {


				enemyID += 1;


				if (Settings::Esp_Corner_Box)
				{
					DrawCornerBox(Head.x - (CornerWidth / 2), Head.y, CornerWidth, CornerHeight, ImGui::ColorConvertFloat4ToU32(BoxColor), Settings::Thickness);
				}

				if (Settings::Esp_line)
				{
					ImGui::GetForegroundDrawList()->AddLine(ImVec2(Width / 2, Height / 2), ImVec2(Head.x, Head.y), ImColor(255, 255, 255, 255), Settings::Thickness);

				}


				if (Settings::skeleton1)
				{

					Vector2 bottom1 = ProjectWorldToScreen(Vector2(bone0.x + 40, bone0.y - 40, bone0.z));
					Vector2 bottom2 = ProjectWorldToScreen(Vector2(bone0.x - 40, bone0.y - 40, bone0.z));
					Vector2 bottom3 = ProjectWorldToScreen(Vector2(bone0.x - 40, bone0.y + 40, bone0.z));
					Vector2 bottom4 = ProjectWorldToScreen(Vector2(bone0.x + 40, bone0.y + 40, bone0.z));
					Vector2 top1 = ProjectWorldToScreen(Vector2(vHeadBone.x + 40, vHeadBone.y - 40, vHeadBone.z + 15));
					Vector2 top2 = ProjectWorldToScreen(Vector2(vHeadBone.x - 40, vHeadBone.y - 40, vHeadBone.z + 15));
					Vector2 top3 = ProjectWorldToScreen(Vector2(vHeadBone.x - 40, vHeadBone.y + 40, vHeadBone.z + 15));
					Vector2 top4 = ProjectWorldToScreen(Vector2(vHeadBone.x + 40, vHeadBone.y + 40, vHeadBone.z + 15));
					ImGui::GetWindowDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(top1.x, top1.y), ESPSkeleton, 0.1f);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(top2.x, top2.y), ESPSkeleton, 0.1f);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(top3.x, top3.y), ESPSkeleton, 0.1f);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(top4.x, top4.y), ESPSkeleton, 0.1f);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(bottom1.x, bottom1.y), ImVec2(bottom2.x, bottom2.y), ESPSkeleton, 0.1f);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(bottom2.x, bottom2.y), ImVec2(bottom3.x, bottom3.y), ESPSkeleton, 0.1f);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(bottom3.x, bottom3.y), ImVec2(bottom4.x, bottom4.y), ESPSkeleton, 0.1f);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(bottom4.x, bottom4.y), ImVec2(bottom1.x, bottom1.y), ESPSkeleton, 0.1f);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(top1.x, top1.y), ImVec2(top2.x, top2.y), ESPSkeleton, 0.1f);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(top2.x, top2.y), ImVec2(top3.x, top3.y), ESPSkeleton, 0.1f);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(top3.x, top3.y), ImVec2(top4.x, top4.y), ESPSkeleton, 0.1f);
					ImGui::GetWindowDrawList()->AddLine(ImVec2(top4.x, top4.y), ImVec2(top1.x, top1.y), ESPSkeleton, 0.1f);


				}





				if (Settings::Distance) {
					char name[64];
					sprintf_s(name, "[%2.fm]", distance);
					DrawString(14, Head.x, Head.y - 15, &Col.white_, true, true, name);
				}
				if (Settings::Esp_line2) {
					ImGui::GetForegroundDrawList()->AddLine(ImVec2(Width / 2, Height / 100), ImVec2(Head.x, Head.y), ImGui::ColorConvertFloat4ToU32(BoxColor), Settings::Thickness);
				}
				if (Settings::Esp_line1)
				{

					ImGui::GetForegroundDrawList()->AddLine(ImVec2(960, 1060), ImVec2(bottom.x, bottom.y), ImGui::ColorConvertFloat4ToU32(BoxColor), 1.0f);


				}
				if (Settings::Head_dot) {
					ImGui::GetForegroundDrawList()->AddCircle(ImVec2(vHeadBoneOu1.x, vHeadBoneOu1.y), 30, ImColor(SkelColor), 2.f);
				}




				if (Settings::Esp_box_fill) {
					ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(Head.x - (CornerWidth / 4), Head.y), ImVec2(bottom.x + (CornerWidth / 4), bottom.y), ImColor(0, 0, 0, 125), 1.0f);
				}



				if (Settings::Esp_box) {
					ImGui::GetForegroundDrawList()->AddRect(ImVec2(Head.x - (CornerWidth / 4), Head.y), ImVec2(bottom.x + (CornerWidth / 4), bottom.y), ImColor(BoxColor), 1.5f);
					

				}
				if (Settings::skeleton) {
					DrawSkeleton(entity.mesh, SkelColor);
				}

				if (Settings::EINA && enemyID > 0)
				{
					std::string s = std::to_string(enemyID);
					std::string z = std::to_string(CloseRangeenemyID);
					std::string i = std::to_string(visennemy);


					std::string a = ("You have ") + s + (" ennemies around you");
					std::string c = ("You have ") + z + (" close range ennemies");
					std::string g = ("You have ") + i + (" visible ennemies");

					auto sizeA = ImGui::CalcTextSize(a.c_str());
					auto sizeC = ImGui::CalcTextSize(c.c_str());
					auto sizeG = ImGui::CalcTextSize(g.c_str());

					DrawNewTextCustom2((float)(Width / 2) - (sizeA.x / 2), 100, 0, 0, 255, 255, a.c_str());
					DrawNewTextCustom2((float)(Width / 2) - (sizeC.x / 2), 120, 255, 255, 255, 255, c.c_str());
					DrawNewTextCustom2((float)(Width / 2) - (sizeG.x / 2), 140, 255, 0, 0, 255, g.c_str());


				}

				if (enemyID != 0)
					enemyID = 0;

				if (CloseRangeenemyID != 0)
					CloseRangeenemyID = 0;


				if (visennemy != 0)
					visennemy = 0;



				if (IsVisible(entity.mesh)) {
					auto dx = aimbotspot.x - (Width / 2);
					auto dy = aimbotspot.y - (Height / 2);
					auto dz = aimbotspot.z - (Depth / 2);
					auto dist = sqrtf(dx * dx + dy * dy + dz * dz) / 100.0f;
					if (dist < Settings::AimFOV) {
						closestDistance = dist;
						closestPawn = entity.Actor;
					}
				}
			}
			if (Settings::Aimbot) {
				if (closestPawn != 0) {
					if (Settings::Aimbot && closestPawn && GetAsyncKeyState(hotkeys::aimkey) < 0) {
						AimAt(closestPawn);
					}
				}
			}

		}
	}


}

namespace fonts {
	ImFont* medium;
	ImFont* semibold;

	ImFont* logo;
}



#include "elements.h"
#include "files/cheat/ImGui/imgui_internal.h"

struct tab_element {
	float element_opacity;
};

bool elements::tab(const char* name)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(name);
	const ImVec2 label_size = ImGui::CalcTextSize(name, NULL, true);
	ImVec2 pos = window->DC.CursorPos;

	const ImRect rect(pos, ImVec2(pos.x + label_size.x, pos.y + label_size.y));
	ImGui::ItemSize(ImVec4(rect.Min.x, rect.Min.y, rect.Max.x + 10, rect.Max.y), style.FramePadding.y);
	if (!ImGui::ItemAdd(rect, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held, NULL);

	static std::map <ImGuiID, tab_element> anim;
	auto it_anim = anim.find(id);
	if (it_anim == anim.end()) {
		anim.insert({ id, { 0.0f } });
		it_anim = anim.find(id);
	}

	it_anim->second.element_opacity = ImLerp(it_anim->second.element_opacity, (0.8f, hovered ? 0.6f : 0.4f), 0.07f * (1.0f - ImGui::GetIO().DeltaTime));

	window->DrawList->AddText(rect.Min, ImColor(1.0f, 1.0f, 1.0f, it_anim->second.element_opacity), name);

	return pressed;
}
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}


LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
			g_d3dpp.BackBufferHeight = HIWORD(lParam);
			ResetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}



LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


void render() {

	ImGui_ImplDX9_NewFrame(); ImGui_ImplWin32_NewFrame(); ImGui::NewFrame();
	RECT rect = { 0 }; if (GetWindowRect(GameWnd, &rect)) { Width = rect.right - rect.left; Height = rect.bottom - rect.top; } ScreenCenterX = (Width / 2.0f), ScreenCenterY = (Height / 2.0f);


	DrawESP();


	if (GetAsyncKeyState(VK_INSERT) & 1) {
		ShowMenu = !ShowMenu;
	}

	if (ShowMenu == true) {



		ImGui::StyleColorsDark();





		enum heads {
			HEAD_1,
			HEAD_2,
			HEAD_3
		};

		static int MenuTab = 0;
		static heads head_selected = HEAD_1;
		static bool checkbox, checkbox_1 = false;

		static int combobox, sliderscalar = 0;
		const char* combobox_items[3] = { "Option 1", "Option 2", "Option 3" };

		ImGui::SetNextWindowSize({ 500, 370 });

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));


		ImGui::Begin("catrine", nullptr, ImGuiWindowFlags_NoDecoration);
		{
			auto draw = ImGui::GetWindowDrawList();

			auto pos = ImGui::GetWindowPos();
			auto size = ImGui::GetWindowSize();

			draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + 51), ImColor(24, 24, 24), 9.0f, ImDrawFlags_RoundCornersTop);
			draw->AddRectFilledMultiColorRounded(pos, ImVec2(pos.x + 55, pos.y + 51), ImColor(1.0f, 1.0f, 1.0f, 0.00f), ImColor(1.0f, 1.0f, 1.0f, 0.05f), ImColor(1.0f, 1.0f, 1.0f, 0.00f), ImColor(1.0f, 1.0f, 1.0f, 0.00f), ImColor(1.0f, 1.0f, 1.0f, 0.05f), 9.0f, ImDrawFlags_RoundCornersTopLeft);

			draw->AddText(fonts::logo, 15.0f, ImVec2(pos.x + 22, pos.y + 17), ImColor(192, 203, 229), ICON_FA_SUN"");
			draw->AddText(fonts::medium, 14.0f, ImVec2(pos.x + 43, pos.y + 18), ImColor(192, 203, 229), "TerminationFN");

			ImGui::SetCursorPos({ 125, 19 });
			ImGui::BeginGroup(); {
				if (elements::tab("Aim"))
					MenuTab = 0;
				ImGui::SameLine();
				if (elements::tab("Visuals"))
					MenuTab = 1;
				ImGui::SameLine();
				if (elements::tab("Misc"))
					MenuTab = 2;
			}
			ImGui::EndGroup();

			if (MenuTab == 0)
			{
				draw->AddText(fonts::medium, 14.0f, ImVec2(pos.x + 25, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.6f), "Aim");

				ImGui::SetCursorPos({ 25, 85 });
				ImGui::BeginChild("##container", ImVec2(190, 275), false, ImGuiWindowFlags_NoScrollbar); {
					ImGui::Checkbox("Aim Assist", &Settings::Aimbot);

					static	int SelectedB = 0;
					static const char* BList[]{ "off","circle", "Square" };
					if (ImGui::Combo("Fov Type", &SelectedB, BList, IM_ARRAYSIZE(BList)))
					{
						if (SelectedB == 0)
						{

							Settings::squarefov = false;
							Settings::Draw_FOV_Circle = false;


						}

						if (SelectedB == 1)
						{


							Settings::squarefov = false;
							Settings::Draw_FOV_Circle = true;


						}
						if (SelectedB == 2)
						{


							Settings::squarefov = true;
							Settings::Draw_FOV_Circle = false;


						}

					}



					ImGui::SliderInt("Aim Fov", &Settings::AimFOV, 0, 100);

					ImGui::Text("Developed by blyscyk#0141");
				}
				ImGui::EndChild();

				draw->AddText(fonts::medium, 14.0f, ImVec2(pos.x + 285, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.6f), "Aim Settings");

				ImGui::SetCursorPos({ 285, 85 });
				ImGui::BeginChild("##container1", ImVec2(190, 275), false, ImGuiWindowFlags_NoScrollbar); {

					static const char* HitboxList[]{ "Head","Chest","Dick" };
					static int SelectedHitbox = 0;

					if (ImGui::Combo("Aimbot Hitbox  ", &SelectedHitbox, HitboxList, IM_ARRAYSIZE(HitboxList)))
					{
						if (SelectedHitbox == 0)//head
						{
							Settings::hitbox = 66;
						}
						if (SelectedHitbox == 1)
						{
							Settings::hitbox = 5;
						}
						if (SelectedHitbox == 2)
						{
							Settings::hitbox = 2;
						}
					}
					ImGui::SliderInt("Aimbot Smoothing", &Settings::Smoothing, 2, 20, "%d%%", ImGuiSliderFlags_NoInput);
					//			ImGui::Checkbox("Crosshair", &Settings::Cross_Hair);
					ImGui::Text(("Aim Key:"));
					ImGui::SameLine();
					HotkeyButton(hotkeys::aimkey, ChangeKey, keystatus);
				}
				ImGui::EndChild();
			}
			if (MenuTab == 1)
			{

				draw->AddText(fonts::medium, 14.0f, ImVec2(pos.x + 25, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.6f), "Visuals");

				ImGui::SetCursorPos({ 25, 85 });
				ImGui::BeginChild("##visual", ImVec2(190, 275), false, ImGuiWindowFlags_NoScrollbar);
				{

					ImGui::Checkbox("VisibleCheck", &Settings::visible);
					ImGui::SameLine();
					ImGui::TextDisabled("(?)");
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
						ImGui::TextUnformatted("If a player is visbile the esp color  will be green and if a player is not visible it will be red ");
						ImGui::PopTextWrapPos();

						ImGui::EndTooltip();
					}


					static const char* BList[]{ "Off", "Box","CornerBox", "FilledBox" };
					static int SelectedB = 0;


					if (ImGui::Combo("Box type  ", &SelectedB, BList, IM_ARRAYSIZE(BList)))
					{
						if (SelectedB == 0)
						{
							Settings::Esp_box = false;
							Settings::Esp_box_fill = false;
							Settings::Esp_Corner_Box = false;

						}

						if (SelectedB == 1)
						{
							Settings::Esp_box = true;
							Settings::Esp_box_fill = false;
							Settings::Esp_Corner_Box = false;
						}

						if (SelectedB == 2)
						{
							Settings::Esp_box = false;
							Settings::Esp_box_fill = false;
							Settings::Esp_Corner_Box = true;
						}

						if (SelectedB == 3)
						{
							Settings::Esp_box = true;
							Settings::Esp_box_fill = true;
							Settings::Esp_Corner_Box = false;
						}
					}







					static const char* SList[]{ "Off", "Bottom", "Middle", "Top" };
					static int SelectedS = 0;

					if (ImGui::Combo("Snapline Type", &SelectedS, SList, IM_ARRAYSIZE(SList)))
					{
						if (SelectedS == 0)
						{
							Settings::Esp_line1 = false;
							Settings::Esp_line = false;
							Settings::Esp_line2 = false;

						}

						if (SelectedS == 1)
						{
							Settings::Esp_line1 = true;
							Settings::Esp_line = false;
							Settings::Esp_line2 = false;
						}
						if (SelectedS == 2)
						{
							Settings::Esp_line1 = false;
							Settings::Esp_line = true;
							Settings::Esp_line2 = true;
						}
						if (SelectedS == 3)
						{
							Settings::Esp_line1 = false;
							Settings::Esp_line = false;
							Settings::Esp_line2 = true;
						}
					}
					ImGui::Checkbox("Head Dot", &Settings::Head_dot);
					ImGui::Checkbox(skCrypt("SkeletonESP"), &Settings::skeleton);
					ImGui::Checkbox("Draw Distance", &Settings::Distance);


					ImGui::Text("Developed by blyscyk#0141");
				}
				ImGui::EndChild();

				draw->AddText(fonts::medium, 14.0f, ImVec2(pos.x + 285, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.6f), "Visual Settings");

				ImGui::SetCursorPos({ 285, 85 });
				ImGui::BeginChild("##visual1", ImVec2(190, 275), false, ImGuiWindowFlags_NoScrollbar);
				{


					ImGui::SliderFloat("Visible Distance", &Settings::VisDist, 10, 500);
					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Text("Colors");
					ImGui::ColorEdit3("Vis Color", visibleesp);
					ImGui::ColorEdit3("InVis Color", invisiboees);
					ImGui::ColorEdit3("Normal Color", normalesp);
				}
				ImGui::EndChild();
			}

		}
		ImGui::End();
	}






	ImGui::EndFrame();


	p_Device->SetRenderState(D3DRS_ZENABLE, false);
	p_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	p_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	p_Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

	if (p_Device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		p_Device->EndScene();
	}

	HRESULT result = p_Device->Present(NULL, NULL, NULL, NULL);

	if (result == D3DERR_DEVICELOST && p_Device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		p_Device->Reset(&p_Params);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}


WPARAM MainLoop()
{
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));

	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, MyWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		HWND hwnd_active = GetForegroundWindow();

		if (hwnd_active == GameWnd) {
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(MyWnd, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		if (GetAsyncKeyState(0x23) & 1)
			exit(8);

		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(GameWnd, &rc);
		ClientToScreen(GameWnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = GameWnd;
		io.DeltaTime = 1.0f / 60.0f;

		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;

		if (GetAsyncKeyState(VK_LBUTTON)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
			io.MouseDown[0] = false;

		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{
			old_rc = rc;

			Width = rc.right;
			Height = rc.bottom;

			p_Params.BackBufferWidth = Width;
			p_Params.BackBufferHeight = Height;
			SetWindowPos(MyWnd, (HWND)0, xy.x, xy.y, Width, Height, SWP_NOREDRAW);
			p_Device->Reset(&p_Params);
		}
		render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyWindow(MyWnd);
}





DWORD processID;




#pragma comment(lib, "ntdll.lib")



struct HandleDisposer
{
	using pointer = HANDLE;
	void operator()(HANDLE handle) const
	{
		if (handle != NULL || handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(handle);
		}
	}
};
using unique_handle = std::unique_ptr<HANDLE, HandleDisposer>;





#include <fstream>
#include <filesystem>
#include <iosfwd>

#include <direct.h>
#include <urlmon.h>
#include "main.h"
#pragma comment(lib, "urlmon.lib")









using unique_handle = std::unique_ptr<HANDLE, HandleDisposer>;
void clear() {
	COORD topLeft = { 0, 0 };
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO screen;
	DWORD written;

	GetConsoleScreenBufferInfo(console, &screen);
	FillConsoleOutputCharacterA(
		console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	FillConsoleOutputAttribute(
		console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
		screen.dwSize.X * screen.dwSize.Y, topLeft, &written
	);
	SetConsoleCursorPosition(console, topLeft);
}

static std::uint32_t _GetProcessId(std::string process_name) {
	PROCESSENTRY32 processentry;
	const unique_handle snapshot_handle(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));

	if (snapshot_handle.get() == INVALID_HANDLE_VALUE)
		return 0;

	processentry.dwSize = sizeof(MODULEENTRY32);

	while (Process32Next(snapshot_handle.get(), &processentry) == TRUE) {
		if (process_name.compare(processentry.szExeFile) == 0)
			return processentry.th32ProcessID;
	}
	return 0;
}


bool initiateCheat() {




	system(XorStr("cls").c_str());
	std::cout << XorStr("\n Loading..").c_str();

	processID = _GetProcessId("FortniteClient-Win64-Shipping.exe");
	KmDrv = new Memory(processID);
	base_address = KmDrv->GetModuleBase("FortniteClient-Win64-Shipping.exe"); (1000);
	system(XorStr("cls").c_str());
	//::ShowWindow(::GetConsoleWindow(), SW_HIDE);
	return true;
}
bool is_file_exist(const char* fileName)
{
	std::ifstream infile(fileName);
	return infile.good();
}


int main()
{

	SetConsoleTitleA(XorStr("  ").c_str());
	system(XorStr("color b").c_str());

	system(XorStr("cls").c_str());
	std::cout << XorStr("\n [TerminationFN] Press F8 to continue. Do this in the main lobby.").c_str();
	while (true)
	{
		Sleep(10);
		if (GetAsyncKeyState(VK_F8))
		{
			break;
		}
	}
	system(XorStr("cls").c_str());
	Beep(500, 500);
	if (!initiateCheat()) {
		system(XorStr("cls").c_str());
		std::cout << XorStr("\n [TerminationFN] Loading error 1 - contact support").c_str();
		Sleep(-1);
	}
	HANDLE handle = CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(drawLoop), nullptr, NULL, nullptr);
	CloseHandle(handle);
	SetupWindow();
	DirectXInit(MyWnd);

	std::cout << "base: " << base_address << std::endl;
	while (true) {
		MainLoop();
	}

	return 0;
}
