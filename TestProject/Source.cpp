#include "TrialLib.h"
#include "../FBX_LIB/FbxModel.h"
#include "../FBX_LIB/FbxRelated.h"


#pragma comment (lib,"DirectX_LIB.lib")
#pragma comment (lib,"libfbxsdk-mt.lib")
#pragma comment (lib,"../Debug/FBX_LIB.lib")

#define _T(p) ((LPCTSTR) p)
#define pCchar(p) ((const char*) p)

#define WIDTH 1600				// クライアントサイズの幅
#define HEIGHT 900 				// クライアントサイズの高さ
#define GAME_FPS (1000 / 60)			// ゲームFPS

#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

float fCameraX = 0.f, fCameraY = 1.0f, fCameraZ = -3.0f,
fCameraHeading = 0.f, fCameraPitch = 0, /*fPosX = 0, fPosY = 0, fPosZ = 0,*/
LightX = 1.f, LightY = 1.f, LightZ = -5.f;
float fPosX = 0.f, fPosY = -1.0f, fPosZ = 0.f, fRoling = 0.f , fHeading = D3DX_PI, fPitch = 0.f,
DragonRoling = 0, DragonHeading = D3DX_PI, DragonPitch = -0.5f * D3DX_PI;


FbxRelated UnityChanRelated;
FbxRelated DragonRelated;

void Render(FbxRelated* fbxRelated);

unsigned int gameRoop();
unsigned int ExitGame();
void Render();
void Control();
void CameraAndLightSetting();


// エントリポイント
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstance, char* szStr, INT iCmdShow) {
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		//_CrtSetBreakAlloc(570826);
		//_CrtSetBreakAlloc(110);
		HWND hWnd = NULL;
		//FbxRelated UnityChanRelated;

		//UNICODEを使用時は第一引数の頭にLをつける
#ifdef _DEBUG
		InitWindowEx(L"☆TEST☆", &hWnd, WIDTH, HEIGHT, hInst, hInstance, pCchar("Blank.jpg"));
#else
		InitWindowFullscreenEx(L"☆TEST☆", &hWnd, WIDTH, HEIGHT, hInst, hInstance, pCchar("Blank.jpg"));
#endif
		// 「DIRECT3Dデバイス」オブジェクトの作成
		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.BackBufferCount = 1;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.Windowed = TRUE;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

		if (FAILED(g_pDirect3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
			D3DCREATE_MIXED_VERTEXPROCESSING,
			&d3dpp, &g_pD3Device)))
		{
			MessageBox(0, _T("HALモードでDIRECT3Dデバイスを作成できません\nREFモードで再試行します"), NULL, MB_OK);
			if (FAILED(g_pDirect3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd,
				D3DCREATE_MIXED_VERTEXPROCESSING,
				&d3dpp, &g_pD3Device)))
			{
				MessageBox(0, _T("DIRECT3Dデバイスの作成に失敗しました"), NULL, MB_OK);
				return E_FAIL;
			}
		}

		UnityChanRelated.LoadFbx("UnityChan/Models/unitychan.FBX");
		//if (false(UnityChanRelated.LoadFbx("./UnityChan/Animations/unitychan_DAMAGED00.FBX"))) {
		//	MessageBox(0, _T("FBXの読込に失敗しました"), NULL, MB_OK);
		//}
		DragonRelated.LoadFbx("FBX/MonsterGroup1/WarFireDragon/Mesh/WarFireDragon.FBX");
		g_pD3Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		g_pD3Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);  //SRCの設定
		g_pD3Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		g_pD3Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		g_pD3Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

		// Zバッファー処理を有効にする
		g_pD3Device->SetRenderState(D3DRS_ZENABLE, TRUE);
		// ライトを有効にする
		g_pD3Device->SetRenderState(D3DRS_LIGHTING, TRUE);
		// アンビエントライト（環境光）を設定する
		g_pD3Device->SetRenderState(D3DRS_AMBIENT, 0x00111111);
		// スペキュラ（鏡面反射）を有効にする
		g_pD3Device->SetRenderState(D3DRS_SPECULARENABLE, false);
		
		ReadInTexture("body_01.tga", "a");
		ReadInTexture("hair_01.tga", "b");
		ReadInTexture("face_00.tga", "c");
		ReadInTexture("cheek_00.tga", "d");
		ReadInTexture("skin_01.tga", "e");
		ReadInTexture("guide.tga", "f");
		ReadInTexture("eye_iris_L_00.tga", "g");
		ReadInTexture("eye_iris_R_00.tga", "h");
		ReadInTexture("eyeline_00.tga", "i");


		FlameRoop(gameRoop);
		//while (!InputKEY(DIK_RETURN)) {
		//	Render(&UnityChanRelated);
		//}
		return 0;
}
void UnityChanRendring() {
	static float gotoaway = -1.0f;
	gotoaway += 0.001;

	for (int i = 0; i < UnityChanRelated.m_pModel.size(); i++) {
		D3DXMATRIX			matPosition;	// 位置座標行列
		D3DXMATRIX			m_MatWorld;
		D3DXMatrixIdentity(&m_MatWorld);
		//拡縮
		D3DXMATRIX			m_MatScale;
		float MatScale = 0.01f;
		D3DXMatrixScaling(&m_MatScale, MatScale, MatScale, MatScale);
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &m_MatScale);

		D3DXMatrixTranslation(&matPosition, 0, 0, 0);
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &matPosition);
		// 回転
		D3DXMATRIX matRoling;
		D3DXMATRIX matHeading;
		D3DXMATRIX matPitch;
		static float pitch = 0;
		pitch += D3DX_PI;
		//D3DXMatrixRotationY(&matPitch, pitch);
		D3DXMatrixRotationZ(&matRoling, fRoling);
		D3DXMatrixRotationY(&matHeading, fHeading);
		D3DXMatrixRotationX(&matPitch, fPitch);
		//D3DXMatrixRotationX(&matHeading, 270 * (3.145f / 180.f));
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &matPitch);
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &matHeading);
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &matRoling);

		//移動
		D3DXMatrixTranslation(&matPosition, fPosX, fPosY, fPosZ);
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &matPosition);
		g_pD3Device->SetTransform(D3DTS_WORLD, &m_MatWorld);

		switch (i) {
		case 0:
			g_pD3Device->SetTexture(0, g_pTexture["c"]);//face
			break;
		case 1:
			g_pD3Device->SetTexture(0, g_pTexture["i"]);//目の裏
			break;
		case 2:
			g_pD3Device->SetTexture(0, g_pTexture["h"]);//righteye
			break;
		case 3:
			g_pD3Device->SetTexture(0, g_pTexture["g"]);//lefteye
			break;
		case 4:
			g_pD3Device->SetTexture(0, g_pTexture["c"]);//faceline
			break;
		case 5:
			g_pD3Device->SetTexture(0, g_pTexture["c"]);//faceeyeline
			break;
		case 6:
			g_pD3Device->SetTexture(0, g_pTexture["i"]);//eyeline
			break;
		case 7:
			g_pD3Device->SetTexture(0, g_pTexture["b"]);//髪
			break;
		case 8:
			g_pD3Device->SetTexture(0, g_pTexture["b"]);//髪
			break;
		case 9:
			g_pD3Device->SetTexture(0, g_pTexture["b"]);//髪
			break;
		case 10:
			g_pD3Device->SetTexture(0, g_pTexture["b"]);//髪
			break;
		case 11:
			g_pD3Device->SetTexture(0, g_pTexture["b"]);//房
			break;
		case 12:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//髪ゴム
			break;
		case 13:
			g_pD3Device->SetTexture(0, g_pTexture["e"]);//bodyskin
			break;
		case 14:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//bodywere
			break;
		case 15:
			g_pD3Device->SetTexture(0, g_pTexture["b"]);//髪
			break;
		case 16:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//west
			break;
		case 17:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//arms
			break;
		case 18:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//upperinnner
			break;
		case 19:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//logo
			break;
		case 20:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//髪飾り
			break;
		case 21:
			g_pD3Device->SetTexture(0, g_pTexture["d"]);//頬
			break;
		case 22:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//legwere
			break;
		case 23:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//
			break;


		}
		UnityChanRelated.m_pModel[i]->DrawFbx();
	}


}
void DragonRendring() {

	for (int i = 0; i < DragonRelated.m_pModel.size(); i++) {
		D3DXMATRIX			matPosition;	// 位置座標行列
		D3DXMATRIX			m_MatWorld;
		D3DXMatrixIdentity(&m_MatWorld);
		//拡縮
		D3DXMATRIX			m_MatScale;
		float MatScale = 1.00f;
		D3DXMatrixScaling(&m_MatScale, MatScale, MatScale, MatScale);
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &m_MatScale);

		// 回転
		D3DXMatrixTranslation(&matPosition, 0, 0, 0);
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &matPosition);

		D3DXMATRIX matRoling;
		D3DXMATRIX matHeading;
		D3DXMATRIX matPitch;
		static float pitch = 0;
		pitch += D3DX_PI;
		//D3DXMatrixRotationY(&matPitch, pitch);
		D3DXMatrixRotationZ(&matRoling, DragonRoling);
		D3DXMatrixRotationY(&matHeading, DragonHeading);
		D3DXMatrixRotationX(&matPitch, DragonPitch);
		//D3DXMatrixRotationX(&matHeading, 270 * (3.145f / 180.f));
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &matPitch);
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &matHeading);
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &matRoling);

		//移動
		D3DXMatrixTranslation(&matPosition, fPosX, fPosY, fPosZ);
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &matPosition);
		g_pD3Device->SetTransform(D3DTS_WORLD, &m_MatWorld);

		DragonRelated.m_pModel[i]->DrawFbx();
	}


}

unsigned int gameRoop() {

	Control();
	Render();


	return ExitGame();
}

unsigned int ExitGame() {
	CheckKeyState(DIK_ESCAPE);
	if (KeyState[DIK_ESCAPE] == KeyRelease)
	{
		return WM_QUIT;
	}
	return WM_NULL;

}

void Render() {
	g_pD3Device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(100, 100, 100), 1.0f, 0);
	g_pD3Device->BeginScene();
	CameraAndLightSetting();
	UnityChanRendring();
	DragonRendring();

	g_pD3Device->SetFVF(D3DFVF_CUSTOMVERTEX);

	EndSetTexture();

}
void Render(FbxRelated* fbxRelated) {
	g_pD3Device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		D3DCOLOR_XRGB(100, 100, 100), 1.0f, 0);
	g_pD3Device->BeginScene();


	for (int i = 0; i < fbxRelated->m_pModel.size(); i++) {
		D3DXMATRIX			m_MatWorld;
		D3DXMatrixIdentity(&m_MatWorld);
		//拡縮
		D3DXMATRIX			m_MatScale;
		float MatScale = 0.01f;
		D3DXMatrixScaling(&m_MatScale, MatScale, MatScale, MatScale);
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &m_MatScale);

		// 回転
		D3DXMATRIX matRoling;
		D3DXMATRIX matHeading;
		D3DXMATRIX matPitch;
		static float pitch = 0;
		pitch += D3DX_PI;
		//D3DXMatrixRotationY(&matPitch, pitch);
		D3DXMatrixRotationZ(&matRoling, fRoling);
		D3DXMatrixRotationY(&matHeading, fHeading + 2.8f);
		D3DXMatrixRotationX(&matPitch, fPitch);
		//D3DXMatrixRotationX(&matHeading, 270 * (3.145f / 180.f));
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &matRoling);
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &matHeading);
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &matPitch);

		//移動
		D3DXMATRIX			matPosition;	// 位置座標行列
		D3DXMatrixTranslation(&matPosition, fPosX, fPosY, fPosZ);
		D3DXMatrixMultiply(&m_MatWorld, &m_MatWorld, &matPosition);
		g_pD3Device->SetTransform(D3DTS_WORLD, &m_MatWorld);

		switch (i) {
		case 0:
			g_pD3Device->SetTexture(0, g_pTexture["c"]);//face
			break;
		case 1:
			g_pD3Device->SetTexture(0, g_pTexture["i"]);//目の裏
			break;
		case 2:
			g_pD3Device->SetTexture(0, g_pTexture["h"]);//righteye
			break;
		case 3:
			g_pD3Device->SetTexture(0, g_pTexture["g"]);//lefteye
			break;
		case 4:
			g_pD3Device->SetTexture(0, g_pTexture["c"]);//faceline
			break;
		case 5:
			g_pD3Device->SetTexture(0, g_pTexture["c"]);//faceeyeline
			break;
		case 6:
			g_pD3Device->SetTexture(0, g_pTexture["i"]);//eyeline
			break;
		case 7:
			g_pD3Device->SetTexture(0, g_pTexture["b"]);//髪
			break;
		case 8:
			g_pD3Device->SetTexture(0, g_pTexture["b"]);//髪
			break;
		case 9:
			g_pD3Device->SetTexture(0, g_pTexture["b"]);//髪
			break;
		case 10:
			g_pD3Device->SetTexture(0, g_pTexture["b"]);//髪
			break;
		case 11:
			g_pD3Device->SetTexture(0, g_pTexture["b"]);//房
			break;
		case 12:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//髪ゴム
			break;
		case 13:
			g_pD3Device->SetTexture(0, g_pTexture["e"]);//bodyskin
			break;
		case 14:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//bodywere
			break;
		case 15:
			g_pD3Device->SetTexture(0, g_pTexture["b"]);//髪
			break;
		case 16:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//west
			break;
		case 17:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//arms
			break;
		case 18:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//upperinnner
			break;
		case 19:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//logo
			break;
		case 20:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//髪飾り
			break;
		case 21:
			g_pD3Device->SetTexture(0, g_pTexture["d"]);//頬
			break;
		case 22:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//legwere
			break;
		case 23:
			g_pD3Device->SetTexture(0, g_pTexture["a"]);//
			break;


		}
		fbxRelated->m_pModel[i]->DrawFbx();
	}


	CameraAndLightSetting();


	EndSetTexture();

}

void Control() {
	//TODO:カメラの向きベクトルからキャラの移動

	//if (InputKEY(DIK_A))fCameraX -= 0.1f;
	//if (InputKEY(DIK_D))fCameraX += 0.1f;
	//if (InputKEY(DIK_Q))fCameraY -= 0.1f;
	//if (InputKEY(DIK_E))fCameraY += 0.1f;
	if (InputKEY(DIK_W)){
		fCameraZ -= 0.1f;
	}
	if (InputKEY(DIK_S)){
		fCameraZ += 0.1f;
	}
	if (InputKEY(DIK_LEFT)) {
		fCameraHeading -= 0.1f;
	}
	if (InputKEY(DIK_RIGHT)) {
		fCameraHeading += 0.1f;
	}
	if (InputKEY(DIK_UP)) {
		fCameraPitch -= 0.1f;
	}
	if (InputKEY(DIK_DOWN)) {
		fCameraPitch += 0.1f;
	}

	if (InputKEY(DIK_J)){
		LightX -= 0.1f;
	}
	if (InputKEY(DIK_L)){
		LightX += 0.1f;
	}
	if (InputKEY(DIK_U)){
		LightY -= 0.1f;
	}
	if (InputKEY(DIK_O)){
		LightY += 0.1f;
	}
	if (InputKEY(DIK_I)){
		LightZ -= 0.1f;
	}
	if (InputKEY(DIK_K)){
		LightZ += 0.1f;
	}
	if (InputKEY(DIK_Y));
	if (InputKEY(DIK_H));
	if (InputKEY(DIK_N));
	if (InputKEY(DIK_M));

	if (InputKEY(DIK_SUBTRACT)) {
		fPosZ -= 0.01f;
	}
	if (InputKEY(DIK_ADD)) {
		fPosZ += 0.01f;
	}
	if (InputKEY(DIK_NUMPAD2)) {
		fPosY -= 0.01f;
	}
	if (InputKEY(DIK_NUMPAD8)) {
		fPosY += 0.01f;
	}
	if (InputKEY(DIK_NUMPAD4)) {
		fPosX -= 0.01f;
	}
	if (InputKEY(DIK_NUMPAD6)) {
		fPosX += 0.01f;
	}
	if (InputKEY(DIK_NUMPAD1)) {
		fHeading -= 0.1f;
		DragonHeading -= 0.1f;
	}
	if (InputKEY(DIK_NUMPAD3)) {
		fHeading += 0.1f;
		DragonHeading += 0.1f;
	}
	if (InputKEY(DIK_NUMPAD7)) {
		fPitch -= 0.1f;
		DragonPitch -= 0.1f;
	}
	if (InputKEY(DIK_NUMPAD9)) {
		fPitch += 0.1f;
		DragonPitch += 0.1f;
	}
	if (InputKEY(DIK_DIVIDE)) {
		fRoling -= 0.1f;
		DragonRoling -= 0.1f;
	}
	if (InputKEY(DIK_MULTIPLY)) {
		fRoling += 0.1f;
		DragonRoling += 0.1f;
	}
}

void CameraAndLightSetting() {
	// ビュートランスフォーム（視点座標変換）
	D3DXVECTOR3 vecEyePt(fCameraX-fPosX, fCameraY- fPosY, fCameraZ- fPosZ); //カメラ（視点）位置
	D3DXVECTOR3 vecLookatPt(fPosX, fPosY, fPosZ);//注視位置
	D3DXVECTOR3 vecUpVec(0.0f, 1.0f, 0.0f);//上方位置
	D3DXMATRIXA16 matView, matCameraPosition, matHeading, matPitch;
	D3DXMatrixIdentity(&matView);
	D3DXMatrixRotationY(&matHeading, fCameraHeading);
	D3DXMatrixRotationX(&matPitch, fCameraPitch);
	D3DXMatrixLookAtLH(&matCameraPosition, &vecEyePt, &vecLookatPt, &vecUpVec);
	D3DXMatrixMultiply(&matView, &matView, &matHeading);
	D3DXMatrixMultiply(&matView, &matView, &matPitch);
	D3DXMatrixMultiply(&matView, &matView, &matCameraPosition);
	g_pD3Device->SetTransform(D3DTS_VIEW, &matView);
	// プロジェクショントランスフォーム（射影変換）
	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 0.1f, 1000.0f);
	g_pD3Device->SetTransform(D3DTS_PROJECTION, &matProj);
	// ライトをあてる 白色で鏡面反射ありに設定
	D3DXVECTOR3 vecDirection(LightX, LightY, LightZ);
	D3DLIGHT9 light;
	ZeroMemory(&light, sizeof(D3DLIGHT9));
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse.r = 1.0f;
	light.Diffuse.g = 1.0f;
	light.Diffuse.b = 1.0f;
	light.Specular.r = 1.0f;
	light.Specular.g = 1.0f;
	light.Specular.b = 1.0f;

	D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDirection);
	light.Range = 200.0f;
	g_pD3Device->SetLight(0, &light);
	g_pD3Device->LightEnable(0, true);

	D3DXVECTOR3 vecDirection1(-LightX, -LightY, -LightZ);
	D3DLIGHT9 light1;
	ZeroMemory(&light1, sizeof(D3DLIGHT9));
	light1.Type = D3DLIGHT_DIRECTIONAL;
	light1.Diffuse.r = 1.0f;
	light1.Diffuse.g = 1.0f;
	light1.Diffuse.b = 1.0f;
	light1.Specular.r = 1.0f;
	light1.Specular.g = 1.0f;
	light1.Specular.b = 1.0f;

	D3DXVec3Normalize((D3DXVECTOR3*)&light1.Direction, &vecDirection1);
	light1.Range = 20.0f;
	g_pD3Device->SetLight(1, &light1);
	g_pD3Device->LightEnable(1, false);

}