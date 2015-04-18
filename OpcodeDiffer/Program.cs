using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Reflection;
using System.Diagnostics;

namespace OpcodeDiffer
{
    class Program
    {
        public static Dictionary<uint, uint> OpcodeToFileOffset = new Dictionary<uint, uint>();

        public static BinaryReader ClientStream { get; private set; }
        public static Stream BaseStream { get { return ClientStream.BaseStream; } }
        public static byte[] ClientBytes { get; private set; }
        public static UnmanagedBuffer Disasm { get; private set; }
        public static BinDiff FuncDiff { get; private set; }
        public static OpcodeTable OpTable { get; private set; }

        static void Main(string[] args)
        {
            if (Config.BinDiff != string.Empty && Config.Opcode != string.Empty)
            {
                Console.WriteLine(">> Opening Diff...");
                FuncDiff = new BinDiff(Config.BinDiff);
                if (!FuncDiff.openConnection())
                {
                    Console.WriteLine(">> Failed to open diff!");
                    return;
                }

                Console.WriteLine(">> Opening OpcodeTable...");
                OpTable = new OpcodeTable(Config.Opcode);
                if (!OpTable.openConnection())
                {
                    Console.WriteLine(">> Failed to open OpcodeTable!");
                    return;
                }
            }
        }
    }
}
