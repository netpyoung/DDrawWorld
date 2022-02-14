using System.Runtime.InteropServices;

namespace DotnetDDrawSample.Utils
{
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct int2
    {
        public int x;
        public int y;

        public int2(in int x, in int y)
        {
            this.x = x;
            this.y = y;
        }
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct int2_rect
    {
        public int2 min;
        public int2 max;

        public int2_rect(in int2 min, in int2 max)
        {
            this.min = min;
            this.max = max;
        }

        public int2_rect(in int min_x, in int min_y, in int max_x, in int max_y)
        {
            this.min = new int2(min_x, min_y);
            this.max = new int2(max_x, max_y);
        }
    }
}