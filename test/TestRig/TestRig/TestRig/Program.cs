using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace TestRig
{
    class Program
    {
        static int x;
        static int y;

        static void Main(string[] args)
        {
            bool running = false;
            x = 0;
            y = 1;

            Thread runThread = new Thread(new ThreadStart(GenerateData()));
        }

        void GenerateData()
        {
            
        }

        string getTestValue(int value)
        {
            return (Math.Sin(value) * 100).ToString().PadLeft(5, '0');
        }

        string buildString()
        {
            string baseString = "D11AMX{0}Y{1}S44\r\n";
            return formatString = string.Format(baseString, getTestValue(x), getTestValue(y));
        }
    }
}
