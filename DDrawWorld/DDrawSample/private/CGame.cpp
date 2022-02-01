#include "ddrawsample/precompiled.h"
#include "ddrawlib/CDDraw.h"
#include "ddrawsample/CGame.h"
#include "Util/Util.h"
#include "Util/QueryPerfCounter.h"
#include "Util/CTGAImage.h"
#include "ImageData/CImageData.h"
#include "ddrawsample/GameObject.h"
#include "ddrawsample/CFlightObject.h"
#include "ddrawsample/Const.h"

using namespace Common;

namespace DDrawSample
{
	CGame::CGame()
	{
		m_currFPS = 0;
		ticksPerGameFrame = 1000.0f / m_targetFPS;
		m_prevGameFrameTick = ::GetTickCount64();
		m_prevCounter = Util::QueryPerfCounter::QCGetCounter();
	}

	CGame::~CGame()
	{
		Cleanup();
	}

	bool CGame::Initialize(const ::HWND hwnd)
	{
		m_hWnd = hwnd;
		m_ddraw = std::make_unique<DDrawLib::CDDraw>();
		if (!m_ddraw->Initialize(hwnd))
		{
			return false;
		}

#pragma warning(push)
#pragma warning(disable: 28159)
		::srand(::GetTickCount());
#pragma warning(pop)

		m_backgroundImage = std::make_unique<Util::CTGAImage>();
		if (!m_backgroundImage->Load24BitsTGA("./data/yuki_back.tga", 4))
		{
			assert(false);
			return false;
		}

		{
			std::unique_ptr<Util::CTGAImage> playerImage = std::make_unique<Util::CTGAImage>();
			if (!playerImage->Load24BitsTGA("./data/galaga_player.tga", 4))
			{
				assert(false);
				return false;
			}
			const ::uint32_t playerColorKey = playerImage->GetPixel(0, 0);
			m_playerImageData = std::make_unique<ImageData::CImageData>();
			m_playerImageData->Create(playerImage->GetRawImage(), playerImage->GetWidth(), playerImage->GetHeight(), playerColorKey);
			playerImage = nullptr;
		}
		
		{
			const int screenWidth = m_ddraw->GetWidth();
			const int screenHeight = m_ddraw->GetHeight();
			const int playerPosX = (screenWidth - m_playerImageData->GetWidth()) / 2;
			const int playerPosY = (screenHeight - m_playerImageData->GetHeight()) / 2;
			m_player = GameObject::CreatePlayer(m_playerImageData.get(), playerPosX, playerPosY, Const::DEFAULT_PLAYER_SPEED);
		}
		
		{
			std::unique_ptr<Util::CTGAImage> ammoImage = std::make_unique<Util::CTGAImage>();
			if (!ammoImage->Load24BitsTGA("./data/ammo.tga", 4))
			{
				assert(false);
				return false;
			}
			uint32_t dwAmmoColorKey = ammoImage->GetPixel(0, 0);
			m_ammoImageData = std::make_unique<ImageData::CImageData>();
			m_ammoImageData->Create(ammoImage->GetRawImage(), ammoImage->GetWidth(), ammoImage->GetHeight(), dwAmmoColorKey);
			ammoImage = nullptr;
		}
		
		{
			std::unique_ptr<Util::CTGAImage> enemyImage = std::make_unique<Util::CTGAImage>();
			if (!enemyImage->Load24BitsTGA("./data/galaga_enemy.tga", 4))
			{
				assert(false);
				return false;
			}
			uint32_t dwEnemyColorKey = enemyImage->GetPixel(5, 0);
			m_enemyImageData = std::make_unique<ImageData::CImageData>();
			m_enemyImageData->Create(enemyImage->GetRawImage(), enemyImage->GetWidth(), enemyImage->GetHeight(), dwEnemyColorKey);
			enemyImage = nullptr;
		}
		return true;
	}

	void CGame::Process()
	{
		const ::LARGE_INTEGER currCounter = Util::QueryPerfCounter::QCGetCounter();
		const float elapsedTick = Util::QueryPerfCounter::QCMeasureElapsedTick(currCounter, m_prevCounter);
		const ::ULONGLONG currTick = GetTickCount64();

		if (elapsedTick > ticksPerGameFrame)
		{
			FixPosition();
			OnGameFrame(currTick);
			m_prevGameFrameTick = currTick;
			m_prevCounter = currCounter;
		}
		else
		{
			const float alpha = elapsedTick / ticksPerGameFrame;
			InterpolatePosition(alpha);
		}
		DrawScene();
	}

	void CGame::OnActivate()
	{
		m_input.Set(E_INPUT::LEFT, false);
		m_input.Set(E_INPUT::RIGHT, false);
		m_input.Set(E_INPUT::UP, false);
		m_input.Set(E_INPUT::DOWN, false);
	}

	void CGame::OnUpdateWindowSize()
	{
		if (m_ddraw != nullptr)
		{
			m_ddraw->OnUpdateWindowSize();
		}
	}

	void CGame::OnUpdateWindowPos()
	{
		if (m_ddraw != nullptr)
		{
			m_ddraw->OnUpdateWindowPos();
		}
	}

	void CGame::OnKeyDown(const uint32_t virtualKeyCode, const uint32_t scanCode)
	{
		UNREFERENCED_PARAMETER(scanCode);

		switch (virtualKeyCode)
		{
		case VK_LEFT:
		{
			m_input.Set(E_INPUT::LEFT, true);
		}
		break;
		case VK_RIGHT:
		{
			m_input.Set(E_INPUT::RIGHT, true);
		}
		break;
		case VK_UP:
		{
			m_input.Set(E_INPUT::UP, true);
		}
		break;
		case VK_DOWN:
		{
			m_input.Set(E_INPUT::DOWN, true);
		}
		break;
		case VK_SPACE:
		{
			ShootFromPlayer();
		}
		break;
		case VK_RETURN:
		{

		}
		break;
		default:
			break;
		}
	}

	void CGame::OnKeyUp(const uint32_t virtualKeyCode, const uint32_t scanCode)
	{
		UNREFERENCED_PARAMETER(scanCode);
		switch (virtualKeyCode)
		{
		case VK_LEFT:
		{
			m_input.Set(E_INPUT::LEFT, false);
		}
		break;
		case VK_RIGHT:
		{
			m_input.Set(E_INPUT::RIGHT, false);
		}
		break;
		case VK_UP:
		{
			m_input.Set(E_INPUT::UP, false);
		}
		break;
		case VK_DOWN:
		{
			m_input.Set(E_INPUT::DOWN, false);
		}
		break;
		default:
			break;
		}
	}

	bool CGame::OnSysKeyDown(const uint32_t virtualKeyCode, const uint32_t scanCode, const bool isAltKeyDown)
	{
		UNREFERENCED_PARAMETER(scanCode);

		switch (virtualKeyCode)
		{
		case VK_F9:
		{
			if (isAltKeyDown)
			{
				CaptureBackBuffer("backbuffer.tga");
				return true;
			}
		}
		break;
		default:
			break;
		}
		return false;
	}

	bool CGame::CaptureBackBuffer(const char* const filePath)
	{
		if (m_ddraw == nullptr)
		{
			return false;
		}

		FILE* fp = nullptr;
		const errno_t err = fopen_s(&fp, filePath, "wb");
		if (err != 0)
		{
			return false;
		}

		m_ddraw->CaptureBackBuffer(fp);

		fclose(fp);
		return true;
	}

	void CGame::DrawScene()
	{
		if (m_ddraw->BeginDraw())
		{
			if (m_backgroundImage != nullptr)
			{
				m_ddraw->DrawBitmap(m_backgroundImagePosX, m_backgroundImagePosY, m_backgroundImage->GetWidth(), m_backgroundImage->GetHeight(), m_backgroundImage->GetRawImage());
			}
			else
			{
				m_ddraw->Clear();
			}

			// player
			const int2& playerPos = m_player->GetInterpolatedPos();
			DrawFlightObject(m_player, playerPos);

			// ammo
			for (int i = 0; i < m_curAmmoNum; ++i)
			{
				CFlightObject* const obj = m_ammoList[i];
				const int2& pos = obj->GetInterpolatedPos();
				DrawFlightObject(obj, pos);
			}
			
			// enemy
			for (int i = 0; i < m_curEnemyNum; ++i)
			{
				CFlightObject* const obj = m_enemyList[i];
				const int2& pos = obj->GetInterpolatedPos();
				DrawFlightObject(obj, pos);
			}
			m_ddraw->EndDraw();
		}

		::HDC hdc = nullptr;
		if (m_ddraw->BeginGDI(&hdc))
		{
			m_ddraw->DrawInfo(hdc);
			m_ddraw->EndGDI(hdc);
		}

		m_ddraw->OnDraw();
		m_ddraw->CheckFPS();
	}

	// ---------------------------
	void CGame::InterpolatePosition(const float alpha)
	{
		if (m_player != nullptr)
		{
			m_player->Interpolate(alpha);
		}
		for (int i = 0; i < m_curAmmoNum; ++i)
		{
			m_ammoList[i]->Interpolate(alpha);
		}
		for (int i = 0; i < m_curEnemyNum; ++i)
		{
			m_enemyList[i]->Interpolate(alpha);
		}
	}

	void CGame::FixPosition()
	{
		if (m_player != nullptr)
		{
			m_player->FixPos();
		}
		for (int i = 0; i < m_curAmmoNum; ++i)
		{
			m_ammoList[i]->FixPos();
		}
		for (int i = 0; i < m_curEnemyNum; ++i)
		{
			m_enemyList[i]->FixPos();
		}
	}

	void CGame::OnGameFrame(const ::ULONGLONG curTick)
	{
		const int screenWidth = m_ddraw->GetWidth();
		const int screenHeight = m_ddraw->GetHeight();
		UpdatePlayerPosistion(screenWidth, screenHeight);

		DeleteDestroyedEnemies(curTick);
		ProcessCollision(curTick);

		// update ammo position
		for (int i = 0; i < m_curAmmoNum;)
		{
			CFlightObject*& ammo = m_ammoList[i];
			assert(ammo != nullptr);

			int2 pos = ammo->GetPos();
			if (pos.y >= 0)
			{
				pos.y -= ammo->GetSpeed();
				ammo->SetPos(pos, true);
				i++;
				continue;
			}

			GameObject::DeleteFlightObject(ammo);
			m_curAmmoNum--;
			ammo = m_ammoList[m_curAmmoNum];
			m_ammoList[m_curAmmoNum] = nullptr;
		}

		ProcessEnemies(screenWidth, screenHeight);
	}

	void CGame::UpdatePlayerPosistion(const int screenWidth, const int screenHeight)
	{
		int2 pos = m_player->GetPos();
		const int playerSpeed = m_player->GetSpeed();
		if (m_input.Get(E_INPUT::LEFT))
		{
			pos.x -= playerSpeed;
		}
		if (m_input.Get(E_INPUT::RIGHT))
		{
			pos.x += playerSpeed;
		}
		if (m_input.Get(E_INPUT::UP))
		{
			pos.y -= playerSpeed;
		}
		if (m_input.Get(E_INPUT::DOWN))
		{
			pos.y += playerSpeed;
		}

		if (m_playerImageData != nullptr)
		{
			const int iPlayerImageWidth = m_playerImageData->GetWidth();
			const int iPlayerImageHeight = m_playerImageData->GetHeight();

			if (pos.x < -(iPlayerImageWidth / 2))
			{
				pos.x = -(iPlayerImageWidth / 2);
				m_backgroundImagePosX++;
			}
			if (pos.x > screenWidth - iPlayerImageWidth + (iPlayerImageWidth / 2))
			{
				pos.x = screenWidth - iPlayerImageWidth + (iPlayerImageWidth / 2);
				m_backgroundImagePosX--;
			}

			if (pos.y < -(iPlayerImageHeight / 2))
			{
				pos.y = -(iPlayerImageHeight / 2);
				m_backgroundImagePosY++;
			}
			if (pos.y > screenHeight - iPlayerImageHeight + (iPlayerImageHeight / 2))
			{
				pos.y = screenHeight - iPlayerImageHeight + (iPlayerImageHeight / 2);
				m_backgroundImagePosY--;
			}
		}

		if (m_backgroundImage != nullptr)
		{
			const int backgroundImageWidth = m_backgroundImage->GetWidth();
			const int backgroundImageHeight = m_backgroundImage->GetHeight();

			if (backgroundImageWidth > screenWidth)
			{
				m_backgroundImagePosX = std::clamp(m_backgroundImagePosX, screenWidth - backgroundImageWidth, 0);
			}
			if (backgroundImageHeight > screenHeight)
			{
				m_backgroundImagePosY = std::clamp(m_backgroundImagePosY, screenHeight - backgroundImageHeight, 0);
			}
		}
		m_player->SetPos(pos, true);
	}

	// ----------------------------------
	void CGame::DrawFlightObject(const CFlightObject* const flight, const int2& pos)
	{
		m_ddraw->DrawImageData(pos.x, pos.y, flight->GetImageData());
	}

	void CGame::ShootFromPlayer()
	{
		if (m_curAmmoNum >= Const::MAX_AMMO_NUM)
		{
			return;
		}

		CFlightObject* const ammo = GameObject::CreateAmmo(m_player, m_ammoImageData.get(), Const::DEFAULT_AMMO_SPEED);
		m_ammoList[m_curAmmoNum] = ammo;
		m_curAmmoNum++;
	}

	void CGame::Cleanup()
	{
		DeleteAllEnemies();
		DeleteAllAmmos();
		if (m_player != nullptr)
		{
			GameObject::DeleteFlightObject(m_player);
			m_player = nullptr;
		}
	}

	void CGame::DeleteAllEnemies()
	{
		for (int i = 0; i < m_curEnemyNum; ++i)
		{
			GameObject::DeleteFlightObject(m_enemyList[i]);
			m_enemyList[i] = nullptr;
		}
		m_curEnemyNum = 0;
	}

	void CGame::DeleteAllAmmos()
	{
		for (int i = 0; i < m_curAmmoNum; ++i)
		{
			GameObject::DeleteFlightObject(m_ammoList[i]);
			m_ammoList[i] = nullptr;
		}
		m_curAmmoNum = 0;
	}

	// ----------
	void CGame::DeleteDestroyedEnemies(const ::ULONGLONG CurTick)
	{
		for (int i = 0; i < m_curEnemyNum;)
		{
			const CFlightObject* const enemy = m_enemyList[i];
			assert(enemy != nullptr);
			const bool isDeath = enemy->GetStatus() == FLIGHT_OBJECT_STATUS::DEAD;
			const bool isDeathEnoughTime = (CurTick > (enemy->GetDeadTick() + Const::DEAD_STATUS_WAIT_TICK));
			if (!isDeath || !isDeathEnoughTime)
			{
				i++;
				continue;
			}

			GameObject::DeleteFlightObject(enemy);
			m_curEnemyNum--;
			m_enemyList[i] = m_enemyList[m_curEnemyNum];
			m_enemyList[m_curEnemyNum] = nullptr;
		}
	}

	void CGame::ProcessCollision(const ::ULONGLONG CurTick)
	{
		for (int i = 0; i < m_curAmmoNum;)
		{
			const CFlightObject* const pAmmo = m_ammoList[i];
			if (!ProcessCollisionAmmoVsEnemies(pAmmo, CurTick))
			{
				i++;
				continue;
			}
				
			GameObject::DeleteFlightObject(pAmmo);
			m_curAmmoNum--;
			m_ammoList[i] = m_ammoList[m_curAmmoNum];
			m_ammoList[m_curAmmoNum] = nullptr;
		}
	}
	void CGame::ProcessEnemies(const int screenWidth, const int screenHeight)
	{
		for (int i = 0; i < m_curEnemyNum;)
		{
			const CFlightObject* const pEnemy = m_enemyList[i];
			const int2& pos = pEnemy->GetPos();
			if (pos.y < screenHeight)
			{
				i++;
				continue;
			}
			GameObject::DeleteFlightObject(pEnemy);
			m_curEnemyNum--;
			m_enemyList[i] = m_enemyList[m_curEnemyNum];
			m_enemyList[m_curEnemyNum] = nullptr;
		}
		MoveEnemies(screenWidth, screenHeight);
		FillEnemies(screenWidth, screenHeight);
	}

	void CGame::MoveEnemies(const int screenWidth, const int screenHeight)
	{
		UNREFERENCED_PARAMETER(screenHeight);

		static ::ULONGLONG PrvEnemyMoveTick = 0;
		ULONGLONG CurTick = GetTickCount64();
		if (CurTick - PrvEnemyMoveTick < Const::ENEMY_MOVE_ACTION_DELAY_TICK)
		{
			return;
		}
		PrvEnemyMoveTick = CurTick;

		for (int i = 0; i < m_curEnemyNum; ++i)
		{
			GameObject::MoveEmemy(m_enemyList[i], screenWidth);
		}
	}

	void CGame::FillEnemies(const int screenWidth, const int screenHeight)
	{
		if (m_curEnemyNum >= Const::MAX_ENEMY_NUM)
		{
			return;
		}

		static ::ULONGLONG s_PrvFillEnemyTick = 0;
		const ::ULONGLONG curTick = GetTickCount64();

		if (curTick - s_PrvFillEnemyTick < 3000)
		{
			return;
		}
		s_PrvFillEnemyTick = curTick;

		const int newEnemyNum = Const::MAX_ENEMY_NUM - m_curEnemyNum;
		for (int i = 0; i < newEnemyNum; ++i)
		{
			CFlightObject* const enemy = GameObject::CreateEnemyRandom(m_enemyImageData.get(), screenWidth, screenHeight, Const::DEFAULT_ENEMY_SPEED);
			m_enemyList[m_curEnemyNum] = enemy;
			m_curEnemyNum++;
		}
	}
	bool CGame::ProcessCollisionAmmoVsEnemies(const CFlightObject* const pAmmo, const ::ULONGLONG curTick)
	{
		for (int i = 0; i < m_curEnemyNum; ++i)
		{
			CFlightObject* pEnemy = m_enemyList[i];
			if (IsCollisionFlightObjectVsFlightObject(pAmmo, pEnemy))
			{
				OnHitEnemy(pEnemy, curTick);
				return true;
			}
		}
		return false;
	}
	void CGame::OnHitEnemy(CFlightObject* const pEnemy, const ::ULONGLONG curTick)
	{
		GameObject::ChangeFlightObjectStatusToDead(pEnemy, curTick);
		AddScore(Const::SCORE_PER_ONE_KILL);
	}

	int CGame::AddScore(const int scoreAdd)
	{
		UNREFERENCED_PARAMETER(scoreAdd);
		return 0;
	}

	bool CGame::IsCollisionFlightObjectVsFlightObject(const CFlightObject* const pObj0, const CFlightObject* const pObj1)
	{
		if (pObj0->GetStatus() != FLIGHT_OBJECT_STATUS::ALIVE || pObj1->GetStatus() != FLIGHT_OBJECT_STATUS::ALIVE)
		{
			return false;
		}

		const int2& ivPos0 = pObj0->GetPos();
		const int2& ivPos1 = pObj1->GetPos();

		int2_rect objRect0 =
		{
			ivPos0.x, ivPos0.y,
			ivPos0.x + pObj0->GetWidth(), ivPos0.y + pObj0->GetHeight()
		};

		int2_rect objRect1 =
		{
			ivPos1.x, ivPos1.y,
			ivPos1.x + pObj1->GetWidth(), ivPos1.y + pObj1->GetHeight()
		};

		const bool isCollision = Util::IsCollisionRectVsRect(&objRect0.min, &objRect0.max, &objRect1.min, &objRect1.max);
		return isCollision;
	}
} // DDrawSample