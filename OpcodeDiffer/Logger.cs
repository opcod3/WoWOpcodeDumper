using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace OpcodeDiffer
{
    class Logger
    {
        private static StreamWriter outputFile = null;

        public static bool CreateOutputStream(string fileName, bool force = false)
        {
            if (fileName == string.Empty)
                return false;

            if (force)
                outputFile.Close();
            outputFile = new StreamWriter(File.Create(fileName));
            return outputFile != null;
        }

        public static void PrepOutputStram()
        {
            outputFile.AutoFlush = true;
        }

        public static void WriteConsoleLine(string str, params object[] obj)
        {
            Console.WriteLine(str, obj);
        }
        
        public static void WriteLine()
        {
            WriteLine(String.Empty);
        }

        public static void WriteLine(string str, params object[] obj)
        {
            Console.WriteLine(str, obj);
            if (outputFile != null)
                outputFile.WriteLine(str, obj);
        }
    }
}
