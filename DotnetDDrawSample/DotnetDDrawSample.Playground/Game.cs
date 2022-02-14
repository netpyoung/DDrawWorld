using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using HDC = TerraFX.Interop.Windows.HDC;
using HWND = TerraFX.Interop.Windows.HWND;
using VK = TerraFX.Interop.Windows.VK;
using DotnetDDrawSample.Utils;
using DotnetDDrawSample.ImageData;
using DotnetDDrawSample.DDrawLib;

namespace DotnetDDrawSample.Playground
{
    public partial class Game
    {
        readonly Timer _timer = new Timer();
        readonly DDraw _ddraw = new DDraw();
        float _targetMillisecondsForFixedUpdate = 0;

        int _renderCurrFPS = 0; // FramesPerSecond
        int _renderPrevFPS = 0;
        long _renderRecalculateFPSlastTick = 0;
        int _renderFrameCount = 0;
        private long _prevQueryPerformanceCount = 0;
        private bool _isDDrawInitialized = false;
        public string _txt_FPS = string.Empty;
        bool[] isKeyDowns = new bool[4];

        private Util.TGAImage _backgroundImage = new Util.TGAImage();
        private ImageData.ImageData _playerImageData = new ImageData.ImageData();
        private ImageData.ImageData _ammoImageData = new ImageData.ImageData();
        private ImageData.ImageData _enemyImageData = new ImageData.ImageData();

        private FlightObject _player = null;
        private int _currEnemyNum = 0;
        private int _currAmmoNum = 0;
        FlightObject[] _ammoList = new FlightObject[Const.MAX_AMMO_NUM];
        FlightObject[] _enemyList = new FlightObject[Const.MAX_ENEMY_NUM];

        private int _backgroundImagePosX = 0;
        private int _backgroundImagePosY = 0;
        private int _screenWidth = 0;
        private int _screenHeight = 0;
        internal bool Initialize(HWND hwnd)
        {
            _targetMillisecondsForFixedUpdate = _timer.MillisecondsPerFrame(60);
            _timer.ElapsedMilliseconds(0, out long currQueryPerformanceCount);
            _prevQueryPerformanceCount = currQueryPerformanceCount;
            _isDDrawInitialized = _ddraw.InitializeDDraw(hwnd);
            if (!_isDDrawInitialized)
            {
                return false;
            }

            _screenWidth = _ddraw.Width;
            _screenHeight = _ddraw.Height;


            if (!_backgroundImage.Load24BitsTGA("./data/yuki_back.tga", 4))
            {
                return false;
            }

            Util.TGAImage playerImage = new Util.TGAImage();
            if (!playerImage.Load24BitsTGA("./data/galaga_player.tga", 4))
            {
                return false;
            }
            UInt32 dwPlayerColorKey = playerImage.GetPixel(0, 0);
            _playerImageData.Create(playerImage.RawImage, playerImage.Width, playerImage.Height, dwPlayerColorKey);

            Util.TGAImage ammoImage = new Util.TGAImage();
            if (!ammoImage.Load24BitsTGA("./data/ammo.tga", 4))
            {
                return false;
            }
            UInt32 dwAmmoColorKey = ammoImage.GetPixel(0, 0);
            _ammoImageData.Create(ammoImage.RawImage, ammoImage.Width, ammoImage.Height, dwAmmoColorKey);

            Util.TGAImage enemyImage = new Util.TGAImage();
            if (!enemyImage.Load24BitsTGA("./data/galaga_enemy.tga", 4))
            {
                return false;
            }
            UInt32 dwEnemyColorKey = enemyImage.GetPixel(5, 0);
            _enemyImageData.Create(enemyImage.RawImage, enemyImage.Width, enemyImage.Height, dwEnemyColorKey);


            int playerPosX = (_screenWidth - _playerImageData.Width) / 2;
            int playerPosY = (_screenHeight - _playerImageData.Height) / 2;
            _player = GameObject.CreatePlayer(_playerImageData, playerPosX, playerPosY, Const.DEFAULT_PLAYER_SPEED);

            return true;
        }

        internal void OnKeyDown(uint vkCode, uint scanCode)
        {
            switch (vkCode)
            {
                case VK.VK_LEFT:
                    isKeyDowns[(int)E_KEY.LEFT] = true;
                    break;
                case VK.VK_RIGHT:
                    isKeyDowns[(int)E_KEY.RIGHT] = true;
                    break;
                case VK.VK_UP:
                    isKeyDowns[(int)E_KEY.UP] = true;
                    break;
                case VK.VK_DOWN:
                    isKeyDowns[(int)E_KEY.DOWN] = true;
                    break;
                case VK.VK_SPACE:
                    ShootFromPlayer();
                    break;
                case VK.VK_RETURN:
                    break;
                default:
                    break;
            }
        }

        private void ShootFromPlayer()
        {
            if (_currAmmoNum >= Const.MAX_AMMO_NUM)
            {
                return;
            }

            FlightObject ammo = GameObject.CreateAmmo(_player, _ammoImageData, Const.DEFAULT_AMMO_SPEED);
            _ammoList[_currAmmoNum] = ammo;
            _currAmmoNum++;
        }

        internal void OnKeyUp(uint vkCode, uint scanCode)
        {
            switch (vkCode)
            {
                case VK.VK_LEFT:
                    isKeyDowns[(int)E_KEY.LEFT] = false;
                    break;
                case VK.VK_RIGHT:
                    isKeyDowns[(int)E_KEY.RIGHT] = false;
                    break;
                case VK.VK_UP:
                    isKeyDowns[(int)E_KEY.UP] = false;
                    break;
                case VK.VK_DOWN:
                    isKeyDowns[(int)E_KEY.DOWN] = false;
                    break;
                default:
                    break;
            }
        }

        internal void OnUpdateWindowPos()
        {
            if (!_isDDrawInitialized)
            {
                return;
            }
            _ddraw.OnUpdateWindowSize();
        }

        internal void OnUpdateWindowSize()
        {
            if (!_isDDrawInitialized)
            {
                return;
            }
            _ddraw.OnUpdateWindowPos();
        }

        internal bool OnSysKeyDown(uint vkCode, uint scanCode, bool isAltKeyDown)
        {
            switch (vkCode)
            {
                case VK.VK_F9:
                    {
                        if (isAltKeyDown)
                        {
                            CaptureBackBuffer("backbuffer.tga");
                            Console.WriteLine("Done");
                            return true;
                        }
                    }
                    break;
                default:
                    break;
            }
            return false;
        }

        private void CaptureBackBuffer(string fpath)
        {
            if (_ddraw == null)
            {
                return;
            }

            using (FileStream f = File.Open(fpath, FileMode.Create))
            {
                _ddraw.CaptureBackBuffer(f);
            }
        }

        internal void OnLostFocus()
        {
            isKeyDowns[(int)E_KEY.LEFT] = false;
            isKeyDowns[(int)E_KEY.RIGHT] = false;
            isKeyDowns[(int)E_KEY.UP] = false;
            isKeyDowns[(int)E_KEY.DOWN] = false;
        }

        #region Process
        internal void Process()
        {
            float elapsedMilliseconds = _timer.ElapsedMilliseconds(_prevQueryPerformanceCount, out long currQueryPerformanceCount);
            ulong currTick = _timer.GetTick();

            if (elapsedMilliseconds > _targetMillisecondsForFixedUpdate)
            {
                Process_PositionFix();
                Process_UpdateFrame(currTick);
                _prevQueryPerformanceCount = currQueryPerformanceCount;
            }
            else
            {
                float alpha = (float)elapsedMilliseconds / _targetMillisecondsForFixedUpdate;
                Process_PositionInterpolate(alpha);
            }
            Process_Render();
        }

        private void Process_Render()
        {
            if (_ddraw.BeginDraw())
            {
                //_ddraw.Clear();
                if (_backgroundImage != null)
                {
                    _ddraw.DrawBitmap(_backgroundImagePosX, _backgroundImagePosY, _backgroundImage.Width, _backgroundImage.Height, _backgroundImage.RawImage);
                }

                int2 playerPos = _player.GetInterpolatedPos();
                DrawFlightObject(_player, playerPos);

                // ammo
                for (int i = 0; i < _currAmmoNum; ++i)
                {
                    FlightObject obj = _ammoList[i];
                    int2 pos = obj.GetInterpolatedPos();
                    DrawFlightObject(obj, pos);
                }

                // enemy
                for (int i = 0; i < _currEnemyNum; ++i)
                {
                    FlightObject obj = _enemyList[i];
                    int2 pos = obj.GetInterpolatedPos();
                    DrawFlightObject(obj, pos);
                }
                _ddraw.EndDraw();
            }

            if (_ddraw.BeginGDI(out HDC hdc))
            {
                //_ddraw.DrawInfo(hdc);
                _ddraw.WriteText(_txt_FPS, 0, 0, 0xffff00FF, hdc);
                _ddraw.EndGDI(hdc);
            }
            _ddraw.OnDraw();

            Render_CheckAndDisplayFPS();
        }

        private void Process_PositionFix()
        {
            if (_player != null)
            {
                _player.FixPosition();
            }
            for (int i = 0; i < _currAmmoNum; ++i)
            {
                _ammoList[i].FixPosition();
            }
            for (int i = 0; i < _currEnemyNum; ++i)
            {
                _enemyList[i].FixPosition();
            }
        }

        private void Process_PositionInterpolate(float alpha)
        {
            if (_player != null)
            {
                _player.InterpolatePosition(alpha);
            }
            for (int i = 0; i < _currAmmoNum; ++i)
            {
                _ammoList[i].InterpolatePosition(alpha);
            }
            for (int i = 0; i < _currEnemyNum; ++i)
            {
                _enemyList[i].InterpolatePosition(alpha);
            }
        }

        private void Process_UpdateFrame(ulong currTick)
        {
            int screenWidth = _ddraw.Width;
            int screenHeight = _ddraw.Height;

            UpdateFrame_PlayerPos(screenWidth, screenHeight);
            UpdateFrame_DeleteDestroyedEnemies(currTick);
            UpdateFrame_ProcessCollision(currTick);
            UpdateFrame_ProcessAmmo();
            UpdateFrame_ProcessEnemy(screenWidth, screenHeight);
        }
        #endregion // Process

        #region UpdateFrame_

        private void UpdateFrame_PlayerPos(in int screenWidth, in int screenHeight)
        {
            if (_player == null)
            {
                return;
            }
            int2 ivPlayerPos = _player.GetPos();

            if (isKeyDowns[(int)E_KEY.LEFT])
            {
                ivPlayerPos.x -= _player.GetSpeed();
            }
            if (isKeyDowns[(int)E_KEY.RIGHT])
            {
                ivPlayerPos.x += _player.GetSpeed();
            }
            if (isKeyDowns[(int)E_KEY.UP])
            {
                ivPlayerPos.y -= _player.GetSpeed();
            }
            if (isKeyDowns[(int)E_KEY.DOWN])
            {
                ivPlayerPos.y += _player.GetSpeed();
            }

            if (_playerImageData != null)
            {
                int iPlayerImageWidth = _playerImageData.Width;
                int iPlayerImageHeight = _playerImageData.Height;

                if (ivPlayerPos.x < -(iPlayerImageWidth / 2))
                {
                    ivPlayerPos.x = -(iPlayerImageWidth / 2);
                    _backgroundImagePosX++;

                }
                if (ivPlayerPos.x > screenWidth - iPlayerImageWidth + (iPlayerImageWidth / 2))
                {
                    ivPlayerPos.x = screenWidth - iPlayerImageWidth + (iPlayerImageWidth / 2);
                    _backgroundImagePosX--;
                }

                if (ivPlayerPos.y < -(iPlayerImageHeight / 2))
                {
                    ivPlayerPos.y = -(iPlayerImageHeight / 2);
                    _backgroundImagePosY++;
                }
                if (ivPlayerPos.y > screenHeight - iPlayerImageHeight + (iPlayerImageHeight / 2))
                {
                    ivPlayerPos.y = screenHeight - iPlayerImageHeight + (iPlayerImageHeight / 2);
                    _backgroundImagePosY--;
                }
            }

            if (_backgroundImage.RawImage != null)
            {
                int iBackImageWidth = _backgroundImage.Width;
                int iBackImageHeight = _backgroundImage.Width;

                if (iBackImageWidth > _screenWidth)
                {
                    if (_backgroundImagePosX > 0)
                    {
                        _backgroundImagePosX = 0;
                    }
                    if (_backgroundImagePosX < _screenWidth - iBackImageWidth)
                    {
                        _backgroundImagePosX = _screenWidth - iBackImageWidth;
                    }
                }
                if (iBackImageHeight > _screenHeight)
                {
                    if (_backgroundImagePosY > 0)
                    {
                        _backgroundImagePosY = 0;
                    }
                    if (_backgroundImagePosY < _screenHeight - iBackImageHeight)
                    {
                        _backgroundImagePosY = _screenHeight - iBackImageHeight;
                    }
                }
            }
            _player.SetPos(ivPlayerPos.x, ivPlayerPos.y, true);
        }

        private void UpdateFrame_DeleteDestroyedEnemies(in ulong currTick)
        {
            for (int i = 0; i < _currEnemyNum;)
            {
                FlightObject enemy = _enemyList[i];
                if (!enemy.IsDeath || !enemy.IsDeathEnoughTime(currTick))
                {
                    i++;
                    continue;
                }

                GameObject.DeleteFlightObject(enemy);
                _currEnemyNum--;
                _enemyList[i] = _enemyList[_currEnemyNum];
                _enemyList[_currEnemyNum] = null;
            }
        }

        private void UpdateFrame_ProcessCollision(in ulong currTick)
        {
            for (int i = 0; i < _currAmmoNum;)
            {
                FlightObject ammo = _ammoList[i];
                if (!ProcessCollisionAmmoVsEnemies(ammo, currTick))
                {
                    i++;
                    continue;
                }
                GameObject.DeleteFlightObject(ammo);
                _currAmmoNum--;
                _ammoList[i] = _ammoList[_currAmmoNum];
                _ammoList[_currAmmoNum] = null;
            }
        }

        private bool ProcessCollisionAmmoVsEnemies(in FlightObject ammo, in ulong currTick)
        {
            for (int i = 0; i < _currEnemyNum; ++i)
            {
                FlightObject enemy = _enemyList[i];
                if (!IsCollisionFlightObjectVsFlightObject(ammo, enemy))
                {
                    continue;
                }

                OnHitEnemy(enemy, currTick);
                return true;
            }
            return false;
        }

        private void OnHitEnemy(in FlightObject enemy, in ulong currTick)
        {
            GameObject.ChangeFlightObjectStatusToDead(enemy, currTick);
            AddScore(Const.SCORE_PER_ONE_KILL);
        }

        private void AddScore(in int score)
        {
            Console.WriteLine($"+ {score}");
        }

        private bool IsCollisionFlightObjectVsFlightObject(in FlightObject obj0, in FlightObject obj1)
        {
            if (obj0.GetStatus() != FLIGHT_OBJECT_STATUS.ALIVE || obj1.GetStatus() != FLIGHT_OBJECT_STATUS.ALIVE)
            {
                return false;
            }

            int2 ivPos0 = obj0.GetPos();
            int2 ivPos1 = obj1.GetPos();

            int2_rect objRect0 = new int2_rect(ivPos0.x, ivPos0.y, ivPos0.x + obj0.Width, ivPos0.y + obj0.Height);
            int2_rect objRect1 = new int2_rect(ivPos1.x, ivPos1.y, ivPos1.x + obj1.Width, ivPos1.y + obj1.Height);

            bool isCollision = Util.IsCollisionRectVsRect(objRect0.min, objRect0.max, objRect1.min, objRect1.max);
            return isCollision;
        }

        private void UpdateFrame_ProcessAmmo()
        {
            for (int i = 0; i < _currAmmoNum;)
            {
                FlightObject ammo = _ammoList[i];

                int2 pos = ammo.GetPos();
                if (pos.y >= 0)
                {
                    pos.y -= ammo.GetSpeed();
                    ammo.SetPos(pos.x, pos.y, true);
                    i++;
                    continue;
                }

                GameObject.DeleteFlightObject(ammo);
                _currAmmoNum--;
                _ammoList[i] = _ammoList[_currAmmoNum];
                _ammoList[_currAmmoNum] = null;
            }
        }

        private void UpdateFrame_ProcessEnemy(in int screenWidth, in int screenHeight)
        {
            for (int i = 0; i < _currEnemyNum;)
            {
                FlightObject enemy = _enemyList[i];
                int2 pos = enemy.GetPos();
                if (pos.y < screenHeight)
                {
                    i++;
                    continue;
                }
                GameObject.DeleteFlightObject(enemy);
                _currEnemyNum--;
                _enemyList[i] = _enemyList[_currEnemyNum];
                _enemyList[_currEnemyNum] = null;
            }
            MoveEnemies(screenWidth, screenHeight);
            FillEnemies(screenWidth, screenHeight);
        }

        static long s_PrvEnemyMoveTick = 0;
        static long s_PrvFillEnemyTick = 0;

        private void MoveEnemies(int screenWidth, int screenHeight)
        {
            long currTick = Stopwatch.GetTimestamp();
            if ((currTick - s_PrvEnemyMoveTick) < Const.ENEMY_MOVE_ACTION_DELAY_TICK)
            {
                return;
            }
            s_PrvEnemyMoveTick = currTick;

            for (int i = 0; i < _currEnemyNum; ++i)
            {
                GameObject.MoveEmemy(_enemyList[i], screenWidth);
            }
        }

        private void FillEnemies(int screenWidth, int screenHeight)
        {
            if (_currEnemyNum >= Const.MAX_ENEMY_NUM)
            {
                return;
            }

            long currTick = Stopwatch.GetTimestamp();
            if ((currTick - s_PrvFillEnemyTick) < Const.ENEMY_REFILL_TICKS)
            {
                return;
            }
            s_PrvFillEnemyTick = currTick;

            int newEnemyNum = Const.MAX_ENEMY_NUM - _currEnemyNum;
            for (int i = 0; i < newEnemyNum; ++i)
            {
                FlightObject enemy = GameObject.CreateEnemyRandom(_enemyImageData, screenWidth, screenHeight, Const.DEFAULT_ENEMY_SPEED);
                _enemyList[_currEnemyNum] = enemy;
                _currEnemyNum++;
            }
        }
        #endregion // UpdateFrame_


        private void Render_CheckAndDisplayFPS()
        {
            bool isFPSChanged = CheckFPS();
            if (isFPSChanged)
            {
                Console.WriteLine(_renderCurrFPS);
                int displayFPS = _renderCurrFPS;
                if (displayFPS > 99)
                {
                    displayFPS = 99;
                }
                _txt_FPS = Const.FPS_STRINGS[displayFPS];
            }
        }

        private bool CheckFPS()
        {
            long currTick = Stopwatch.GetTimestamp();
            long diffTick = currTick - _renderRecalculateFPSlastTick;
            if (diffTick < Stopwatch.Frequency)
            {
                _renderFrameCount++;
                return false;
            }

            _renderPrevFPS = _renderCurrFPS;
            _renderCurrFPS = _renderFrameCount;
            _renderRecalculateFPSlastTick = currTick;
            _renderFrameCount = 1;
            if (_renderCurrFPS == _renderPrevFPS)
            {
                return false;
            }

            return true;
        }
        //------------------
        #region GAME
        private void DrawFlightObject(FlightObject player, in int2 pos)
        {
            _ddraw.DrawImageData(pos.x, pos.y, player.GetImageData());
        }

        #endregion // GAME
    }
}
