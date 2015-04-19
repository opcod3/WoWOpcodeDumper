using System;
using System.Collections.Generic;
using System.Net;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace OpcodeDiffer
{
    public class Opcode
    {
        protected Opcode(int opcode)
        {
            this.opcode = opcode;
        }

        public int opcode { get; private set; }
        public string name;
    }

    public class CMSG : Opcode
    {
        public CMSG(int opcode, UInt32 vTable, UInt32 cliPut, UInt32 caller)
            : base(opcode)
        {
            this.vTable = vTable;
            this.cliPut = cliPut;
            this.caller = caller;
        }

        public UInt32 vTable { get; private set; }
        public UInt32 cliPut { get; private set; }
        public UInt32 caller { get; private set; }
    }

    public class SMSG : Opcode
    {
        public SMSG(int opcode, UInt32 ctor, UInt32 callHandler, UInt32 handler)
            : base(opcode)
        {
            this.ctor = ctor;
            this.callHandler = callHandler;
            this.handler = handler;
        }

        public UInt32 ctor { get; private set; }
        public UInt32 callHandler { get; private set; }
        public UInt32 handler { get; private set; }
    }

    public static class Opcodes
    {
        public static Regex OpcodeRgx = new Regex(@"Opcode\.(.+), 0x([A-Z0-9]+)(?: | 0x[0-9A-Z]+)?", RegexOptions.IgnoreCase);

        private static string FilePath = @"https://raw.githubusercontent.com/TrinityCore/WowPacketParser/master/WowPacketParser/Enums/Version/V6_1_2_19802/Opcodes.cs";
        public static bool populateFromWPP()
        {
            Logger.WriteConsoleLine("Loading opcodes from GitHub, build 19802...");
            try
            {
                WebClient client = new WebClient();
                Stream stream = client.OpenRead(FilePath);
                StreamReader reader = new StreamReader(stream);
                var content = reader.ReadToEnd().Split(new string[] { "\r\n", "\n" }, StringSplitOptions.None);
                stream.Close();
                foreach (var line in content)
                {
                    var rgxResult = OpcodeRgx.Match(line);
                    if (!rgxResult.Success)
                        continue;

                    var opcodeName = rgxResult.Groups[1].Value;
                    var opcodeValue = Convert.ToInt32(rgxResult.Groups[2].Value, 16);
                    if (opcodeName.Contains("CMSG"))
                        CMSG_temp.Add(opcodeValue, opcodeName);
                    else
                        SMSG_temp.Add(opcodeValue, opcodeName);
                }
            }
            catch (WebException /*whatever*/) // Haha so funny I is.
            {
                Logger.WriteConsoleLine("Unable to query opcodes. Exiting. Try again.");
                return false;
            }

            return true;
        }

        public static Dictionary<int, string> CMSG_temp = new Dictionary<int, string>();
        public static Dictionary<int, string> SMSG_temp = new Dictionary<int, string>();
    }
}
