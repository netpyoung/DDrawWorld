#include "ddrawsample/precompiled_ddrawsample.h"
#include "ImageData/CImageData.h"
#include "ddrawsample/GameObject.h"
#include "ddrawsample/CFlightObject.h"

namespace DDrawSample::GameObject
{
	CFlightObject* CreatePlayer(ImageData::CImageData* const imageData, const int x, const int y, const int speed)
	{
		CFlightObject* const obj = new CFlightObject();
		obj->SetImageData(imageData);
		obj->SetPos(x, y, false);
		obj->SetStatus(FLIGHT_OBJECT_STATUS::ALIVE);
		obj->SetSpeed(speed);
		return obj;
	}

	CFlightObject* CreateAmmo(CFlightObject* const flightObject, ImageData::CImageData* const pImgData, const int speed)
	{
		CFlightObject* const obj = new CFlightObject();
		obj->SetImageData(pImgData);
		obj->SetStatus(FLIGHT_OBJECT_STATUS::ALIVE);

		const int2& pos = flightObject->GetPos();
		obj->SetPos(pos.x + flightObject->GetWidth() / 2, pos.y, false);
		obj->SetSpeed(speed);

		return obj;
	}

	void DeleteFlightObject(const CFlightObject* const flightObject)
	{
		delete flightObject;
	}

	CFlightObject* CreateEnemyRandom(ImageData::CImageData* const imageData, const int screenWidth, const int screenHeight, const int speed)
	{
		UNREFERENCED_PARAMETER(screenHeight);

		CFlightObject* const obj = new CFlightObject;
		obj->SetImageData(imageData);

		const int pos_x = rand() % (screenWidth - imageData->GetWidth());

		obj->SetPos(pos_x, 0, false);
		obj->SetStatus(FLIGHT_OBJECT_STATUS::ALIVE);
		obj->SetSpeed(speed);
		return obj;
	}

	bool MoveEmemy(CFlightObject* const flightObject, const int screenWidth)
	{
		if (flightObject->GetStatus() != FLIGHT_OBJECT_STATUS::ALIVE)
		{
			return false;
		}

		int2 pos = flightObject->GetPos();
		const int move_x = (rand() % 3) - 2;
		UNREFERENCED_PARAMETER(move_x);
		if (pos.x < 0)
		{
			pos.x = 0;
		}
		const int rightMax = screenWidth - flightObject->GetWidth();
		if (pos.x > rightMax)
		{
			pos.x -= (pos.x - rightMax);
		}
		pos.y += 10;
		flightObject->SetPos(pos, true);
		return true;
	}

	void ChangeFlightObjectStatusToDead(CFlightObject* const flightObject, const ULONGLONG curTick)
	{
		flightObject->SetStatus(FLIGHT_OBJECT_STATUS::DEAD);
		flightObject->SetDeadTick(curTick);
	}
} // DDrawSample::GameObject