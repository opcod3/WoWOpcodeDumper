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

        public static BinDiff funcDiff { get; private set; }
        public static NameDB nameDB { get; private set; }

        static void Main(string[] args)
        {
            //Opcodes.populateFromWPP();
            nameDB = new NameDB(String.Concat(Directory.GetCurrentDirectory(), "/Resources/Names.db"));
            nameDB.OpenConnection();

            Logger.WriteConsoleLine(nameDB.GetNameFromHandlerSMSG(10014047));

            Console.ReadKey();
            //if (Config.BinDiff != string.Empty && Config.Opcode != string.Empty)
            //{
            //    Console.WriteLine(">> Opening Diff...");
            //    funcDiff = new BinDiff(Config.BinDiff);
            //    if (!funcDiff.OpenConnection())
            //    {
            //        Console.WriteLine(">> Failed to open diff!");
            //        return;
            //    }

            //    Console.WriteLine(">> Opening OpcodeTable...");
            //    OpTable = new OpcodeTable(Config.Opcode);
            //    if (!OpTable.OpenConnection())
            //    {
            //        Console.WriteLine(">> Failed to open OpcodeTable!");
            //        return;
            //    }
            //}

        }
    }
}
