using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Net;
using System.IO;

namespace OpcodeDiffer
{
    static class Namer
    {
        public static Regex OpcodeRgx = new Regex(@"Opcode\.(.+), 0x([A-Z0-9]+)(?: | 0x[0-9A-Z]+)?", RegexOptions.IgnoreCase);

        private static string FilePath = @"https://raw.githubusercontent.com/TrinityCore/WowPacketParser/master/WowPacketParser/Enums/Version/V6_1_2_19802/Opcodes.cs";

        //public static bool populateFromWPP()
        //{
        //    Logger.WriteConsoleLine("Loading opcodes from GitHub, build 19802...");
        //    try
        //    {
        //        WebClient client = new WebClient();
        //        Stream stream = client.OpenRead(FilePath);
        //        StreamReader reader = new StreamReader(stream);
        //        var content = reader.ReadToEnd().Split(new string[] { "\r\n", "\n" }, StringSplitOptions.None);
        //        stream.Close();
        //        foreach (var line in content)
        //        {
        //            var rgxResult = OpcodeRgx.Match(line);
        //            if (!rgxResult.Success)
        //                continue;

        //            var opcodeName = rgxResult.Groups[1].Value;
        //            var opcodeValue = Convert.ToInt32(rgxResult.Groups[2].Value, 16);
        //            if (opcodeName.Contains("CMSG"))
        //                CMSG_temp.Add(opcodeValue, opcodeName);
        //            else
        //                SMSG_temp.Add(opcodeValue, opcodeName);
        //        }
        //    }
        //    catch (WebException /*whatever*/) // Haha so funny I is.
        //    {
        //        Logger.WriteConsoleLine("Unable to query opcodes. Exiting. Try again.");
        //        return false;
        //    }

        //    return true;
        //}

        public static void populateFromDiff()
        {
            Console.WriteLine("Diffing names...");
            foreach(CMSG cmsg in Program.LoadedCMSG)
            {
                UInt32 oldCaller = (UInt32)Program.funcDiff.GetAddr2FromAddr1(cmsg.caller);
                if (oldCaller != 0)
                {
                    string name = Program.nameDB.CMSG_GetNameFromCaller(oldCaller);
                    if (name != null)
                        cmsg.name = name;
                }
            }

            foreach(SMSG smsg in Program.LoadedSMSG)
            {
                UInt32 oldHandler = (UInt32)Program.funcDiff.GetAddr2FromAddr1(smsg.handler);
                if (oldHandler != 0)
                {
                    string name = Program.nameDB.SMSG_GetNameFromHandler(oldHandler);
                    if (name != null)
                        smsg.name = name;
                }
            }
        }

        public static Dictionary<int, string> CMSG_temp = new Dictionary<int, string>();
        public static Dictionary<int, string> SMSG_temp = new Dictionary<int, string>();
    }
}
