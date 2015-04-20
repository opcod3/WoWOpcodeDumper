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
        // Max opcode value is NOT static. Could change in big content patches, and expansions.
        public static UInt32 MAX_OPCODE = 0x1FFF;

        public static DiffDB funcDiff { get; private set; }
        public static NameDB nameDB { get; private set; }
        public static DumpDB dumpDB { get; private set; }

        private static List<CMSG> LoadedCMSG = new List<CMSG>(500);
        private static List<SMSG> LoadedSMSG = new List<SMSG>(500); 

        static void Main(string[] args)
        {
            //Opcodes.populateFromWPP();
            nameDB = new NameDB(String.Concat(Directory.GetCurrentDirectory(), "/Resources/Names.db"));

            if (args.Length == 1)
                dumpDB = new DumpDB(args[0]);
            else
                dumpDB = new DumpDB("Dump.db");

            Logger.CreateOutputStream("Dump.txt");
            Logger.PrepOutputStram();

            for (int i = 0; i <= MAX_OPCODE; i++)
            {
                CMSG cmsg = dumpDB.CMSG_Get(i);
                if ( cmsg != null )
                {
                    LoadedCMSG.Add(cmsg);
                }

                SMSG smsg = dumpDB.SMSG_Get(i);
                if ( smsg != null )
                {
                    LoadedSMSG.Add(smsg);
                }

            }

            Logger.WriteLine(LoadedCMSG.First().getTableHeader());
            foreach(CMSG cmsg in LoadedCMSG)
            {
                Logger.WriteLine(cmsg.ToString());
            }

            Logger.WriteLine();
            Logger.WriteLine(LoadedSMSG.First().getTableHeader());
            foreach(SMSG smsg in LoadedSMSG)
            {
                Logger.WriteLine(smsg.ToString());
            }

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
