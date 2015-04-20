using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace OpcodeDiffer
{
    class Program
    {
        // Max opcode value is NOT static. Could change in big content patches, and expansions.
        public static UInt32 MAX_OPCODE = 0x1FFF;

        public static DiffDB funcDiff { get; private set; }
        public static NameDB nameDB { get; private set; }
        public static DumpDB dumpDB { get; private set; }

        public static List<CMSG> LoadedCMSG = new List<CMSG>(500);
        public static List<SMSG> LoadedSMSG = new List<SMSG>(500); 

        static void Main(string[] args)
        {
            //Opcodes.populateFromWPP();
            nameDB = new NameDB(String.Concat(Directory.GetCurrentDirectory(), "/Resources/Names.db"));

            if (args.Length == 1)
                dumpDB = new DumpDB(args[0]);
            else if (args.Length == 2)
            {
                dumpDB = new DumpDB(args[0]);
                funcDiff = new DiffDB(args[1]);
            }
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

            if(funcDiff != null && funcDiff.isOpen())
            {
                Namer.populateFromDiff();
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

            Console.WriteLine("Diff complete! Press any key to close...");
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
