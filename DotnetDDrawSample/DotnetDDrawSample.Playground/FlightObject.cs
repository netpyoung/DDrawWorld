using System;

namespace DotnetDDrawSample.Core
{
    public class FlightObject
    {
        int2 m_oldPos = new int2(0, 0);
        int2 m_curPos = new int2(0, 0);
        int2 m_interpolatedPos = new int2(0, 0);

        ImageData m_imageData = null;
        FLIGHT_OBJECT_STATUS m_status = FLIGHT_OBJECT_STATUS.ALIVE;
        ulong m_deadTick = 0;
        int m_speed = 0;
        public int Width => m_imageData.Width;
        public int Height => m_imageData.Height;

        public bool IsDeath => (m_status == FLIGHT_OBJECT_STATUS.DEAD);
        public bool IsDeathEnoughTime(ulong currTick)
        {
            return (currTick > (m_deadTick + Const.DEAD_STATUS_WAIT_TICK));
        }

        public ulong GetDeadTick()
        {
            return m_deadTick;
        }
        public void SetDeadTick(in ulong deadTick)
        {
            m_deadTick = deadTick;
        }
        public FLIGHT_OBJECT_STATUS GetStatus()
        {
            return m_status;
        }
        public void SetStatus(in FLIGHT_OBJECT_STATUS status)
        {
            m_status = status;
        }
        public ImageData GetImageData()
        {
            return m_imageData;
        }
        public void SetImageData(ImageData imageData)
        {
            m_imageData = imageData;
        }
        public int GetSpeed()
        {
            return m_speed;
        }
        public void SetSpeed(in int iSpeed)
        {
            m_speed = iSpeed;
        }
        public int2 GetInterpolatedPos()
        {
            return m_interpolatedPos;
        }
        public int2 GetPos()
        {
            return m_curPos;
        }

        public void SetPos(in int x, in int y, in bool isInterpolation)
        {
            if (isInterpolation)
            {
                m_oldPos = m_curPos;
            }
            else
            {
                m_oldPos.x = x;
                m_oldPos.y = y;
                m_interpolatedPos.x = x;
                m_interpolatedPos.y = y;
            }
            m_curPos.x = x;
            m_curPos.y = y;
        }

        public void InterpolatePosition(in float inAlpha)
        {
            float alpha = Math.Clamp(inAlpha, 0.0f, 1.0f);
            float x_velocity = (float)(m_curPos.x - m_oldPos.x);
            float y_velocity = (float)(m_curPos.y - m_oldPos.y);

            m_interpolatedPos.x = (int)((float)m_oldPos.x + x_velocity * alpha);
            m_interpolatedPos.y = (int)((float)m_oldPos.y + y_velocity * alpha);
        }

        public void FixPosition()
        {
            m_oldPos = m_curPos;
            m_interpolatedPos = m_curPos;
        }
    }
}
