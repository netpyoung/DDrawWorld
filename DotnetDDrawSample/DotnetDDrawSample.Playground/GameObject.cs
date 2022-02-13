using System;

namespace DotnetDDrawSample.Core
{
    public static class GameObject
    {

        static Random rand = new Random();

        public static FlightObject CreatePlayer(in ImageData imageData, in int x, in int y, in int speed)
        {
            FlightObject obj = new FlightObject();
            obj.SetImageData(imageData);
            obj.SetPos(x, y, false);
            obj.SetStatus(FLIGHT_OBJECT_STATUS.ALIVE);
            obj.SetSpeed(speed);
            return obj;
        }

        public static FlightObject CreateAmmo(in FlightObject flightObject, in ImageData pImgData, in int speed)
        {
            int2 pos = flightObject.GetPos();

            FlightObject obj = new FlightObject();
            obj.SetImageData(pImgData);
            obj.SetStatus(FLIGHT_OBJECT_STATUS.ALIVE);
            obj.SetPos(pos.x + flightObject.Width / 2, pos.y, false);
            obj.SetSpeed(speed);
            return obj;
        }

        public static FlightObject CreateEnemyRandom(in ImageData imageData, in int screenWidth, in int screenHeight, in int speed)
        {
            int pos_x = rand.Next() % (screenWidth - imageData.Width);

            FlightObject obj = new FlightObject();
            obj.SetImageData(imageData);
            obj.SetPos(pos_x, 0, false);
            obj.SetStatus(FLIGHT_OBJECT_STATUS.ALIVE);
            obj.SetSpeed(speed);
            return obj;
        }
        public static void ChangeFlightObjectStatusToDead(in FlightObject flightObject, in ulong curTick)
        {
            flightObject.SetStatus(FLIGHT_OBJECT_STATUS.DEAD);
            flightObject.SetDeadTick(curTick);
        }

        public static bool MoveEmemy(in FlightObject enemy, in int screenWidth)
        {
            if (enemy.GetStatus() != FLIGHT_OBJECT_STATUS.ALIVE)
            {
                return false;
            }

            int2 pos = enemy.GetPos();
            int move_x = (rand.Next() % 3) - 2;
            if (pos.x < 0)
            {
                pos.x = 0;
            }
            int rightMax = screenWidth - enemy.Width;
            if (pos.x > rightMax)
            {
                pos.x -= (pos.x - rightMax);
            }
            pos.y += 10;
            enemy.SetPos(pos.x, pos.y, true);
            return true;
        }

        internal static void DeleteFlightObject(in FlightObject enemy)
        {
        }
    }
}
