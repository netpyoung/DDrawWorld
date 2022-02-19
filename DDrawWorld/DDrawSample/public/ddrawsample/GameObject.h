#pragma once
namespace ImageData
{
	class CImageData;
} // ImageData

namespace DDrawSample
{
	class CFlightObject;
} // DDrawSample

namespace DDrawSample::GameObject
{
	CFlightObject* CreatePlayer(ImageData::CImageData* const pImgData, const int x, const int y, const int iSpeed);
	CFlightObject* CreateEnemyRandom(ImageData::CImageData* const pImgData, const int iScreenWidth, const int iScreenHeight, const int iSpeed);
	CFlightObject* CreateAmmo(CFlightObject* const pShooter, ImageData::CImageData* const pImgData, const int iSpeed);
	void DeleteFlightObject(const CFlightObject* const pObject);
	bool MoveEmemy(CFlightObject* const pObj, const int iScreenWidth);
	void ChangeFlightObjectStatusToDead(CFlightObject* const pObject, const ULONGLONG CurTick);
} // DDrawSample::GameObject