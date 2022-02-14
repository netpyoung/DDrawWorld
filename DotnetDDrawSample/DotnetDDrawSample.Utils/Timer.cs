using System;
using System.Runtime.InteropServices;
using Windows = TerraFX.Interop.Windows.Windows;
using LARGE_INTEGER = TerraFX.Interop.Windows.LARGE_INTEGER;

namespace DotnetDDrawSample.Utils
{
    public class Timer : IDisposable
    {
        readonly long _QueryPerformanceCountsPerSecond;

        public unsafe Timer()
        {
            Windows.timeBeginPeriod(1);
            LARGE_INTEGER lg;
            Windows.QueryPerformanceFrequency(&lg);
            _QueryPerformanceCountsPerSecond = lg.QuadPart;
        }

        public float MillisecondsPerFrame(in int frame)
        {
            return 1000.0f / frame;
        }

        public ulong GetTick()
        {
            return Windows.GetTickCount64();
        }

        public long QueryPerformanceCountsPerFrame(in int frame)
        {
            return _QueryPerformanceCountsPerSecond / frame;
        }

        public unsafe float ElapsedMilliseconds(long prevQueryPerformanceCount, out long currQueryPerformanceCount)
        {
            LARGE_INTEGER lg;
            Windows.QueryPerformanceCounter(&lg);
            currQueryPerformanceCount = lg.QuadPart;
            long elapsedQueryPerformanceCount = currQueryPerformanceCount - prevQueryPerformanceCount;
            float elapsedSeconds = (float)elapsedQueryPerformanceCount / _QueryPerformanceCountsPerSecond;
            float elapsedMilliseconds = elapsedSeconds * 1000.0f;
            return elapsedMilliseconds;
        }

        public void Dispose()
        {
            Windows.timeEndPeriod(1);
        }
    }
}
