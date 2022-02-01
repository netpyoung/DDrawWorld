#pragma once
#include "ddrawsample/precompiled.h"
#include "Common/math.h"
#include "ddrawsample/Const.h"

namespace DDrawLib
{
	class CDDraw;
}

namespace ImageData
{
	class CImageData;
}

namespace Util
{
	class CTGAImage;
}

namespace DDrawSample
{
	enum class E_INPUT
	{
		UP = 0,
		DOWN,
		LEFT,
		RIGHT,
		MAX,
	};

	class CInput final
	{
	private:
		bool m_arr[static_cast<int>(E_INPUT::MAX)] = { false, };
	public:
		void Set(const E_INPUT input, bool isSet)
		{
			assert(input != E_INPUT::MAX);
			m_arr[static_cast<int>(input)] = isSet;
		}

		bool Get(const E_INPUT input) const
		{
			return m_arr[static_cast<int>(input)];
		}
	};
}

using namespace Common;

namespace DDrawSample
{
	class CFlightObject;

	class CGame final
	{
	private:
		::HWND m_hWnd = nullptr;
		std::unique_ptr<DDrawLib::CDDraw> m_ddraw = nullptr;
		CInput m_input = { };
		int m_currFPS = 0;
		int m_targetFPS = 60;
		float ticksPerGameFrame = 0;
		::ULONGLONG m_prevGameFrameTick = 0;
		::LARGE_INTEGER m_prevCounter = { 0, };
		
	private:
		std::unique_ptr<ImageData::CImageData> m_playerImageData = nullptr;
		int m_backgroundImagePosX = 0;
		int m_backgroundImagePosY = 0;
		std::unique_ptr<Util::CTGAImage> m_backgroundImage = nullptr;
		CFlightObject* m_player = nullptr;
		
		int m_curAmmoNum = 0;
		CFlightObject* m_ammoList[Const::MAX_AMMO_NUM] = {0, };
		std::unique_ptr<ImageData::CImageData> m_ammoImageData = nullptr;
		std::unique_ptr<ImageData::CImageData> m_enemyImageData = nullptr;
		int m_curEnemyNum = 0;
		CFlightObject* m_enemyList[Const::MAX_ENEMY_NUM] = { 0, };
	public:
		CGame();
		virtual ~CGame();
	public:
		void Process();
		bool Initialize(const ::HWND hwnd);
		void DrawScene();
	public:
		void OnActivate();
		void OnUpdateWindowSize();
		void OnUpdateWindowPos();
		void OnKeyDown(const uint32_t virtualKeyCode, const uint32_t scanCode);
		void OnKeyUp(const uint32_t virtualKeyCode, const uint32_t scanCode);
		bool OnSysKeyDown(const uint32_t virtualKeyCode, const uint32_t scanCode, const bool isAltKeyDown);
	private:
		void FixPosition();
		void InterpolatePosition(const float alpha);
		void OnGameFrame(const ::ULONGLONG curTick);
		void UpdatePlayerPosistion(const int screenWidth, const int screenHeight);
	private:
		void DrawFlightObject(const CFlightObject* const flight, const int2& pos);
		void ShootFromPlayer();
		void Cleanup();
		void DeleteAllAmmos();
	private:
		bool CaptureBackBuffer(const char* const filePath);
	private:
		void DeleteDestroyedEnemies(const ULONGLONG curTick);
		void ProcessCollision(const ULONGLONG curTick);
		void ProcessEnemies(const int screenWidth, const int screenHeight);
		void MoveEnemies(const int screenWidth, const int screenHeight);
		void FillEnemies(const int screenWidth, const int screenHeight);
		bool ProcessCollisionAmmoVsEnemies(const CFlightObject* const pAmmo, const ULONGLONG curTick);
		void OnHitEnemy(CFlightObject* const pEnemy, const ULONGLONG curTick);
		int AddScore(const int scoreAdd);
		bool IsCollisionFlightObjectVsFlightObject(const CFlightObject* const pObj0, const CFlightObject* const pObj1);
		void DeleteAllEnemies();
	};
} // DDrawSample