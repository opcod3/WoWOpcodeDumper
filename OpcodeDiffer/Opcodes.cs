using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace OpcodeDiffer
{
    abstract public class Opcode
    {
        protected Opcode(int opcode)
        {
            this.opcode = opcode;
        }

        public int opcode { get; private set; }
        public string name;

        public abstract string getTableHeader();
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

		public override string ToString ()
		{
			return string.Format("| 0x{0:X4} |  0x{1:X8}  |  0x{2:X8}  |  0x{3:x8}  | {4}", opcode, vTable, cliPut, caller, name);
		}

		public override string getTableHeader()
		{
			StringBuilder tableHeader = new StringBuilder();
            tableHeader.Append("-------------------------CMSG--------------------------");
            tableHeader.AppendLine();
			tableHeader.Append("+--------+--------------+--------------+--------------+");
			tableHeader.AppendLine ();
			tableHeader.Append("| Opcode |    vTable    |    cliPut    |    caller    |");
			tableHeader.AppendLine ();
			tableHeader.Append("+--------+--------------+--------------+--------------+");			return tableHeader.ToString ();
        }
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

		public override string ToString ()
		{
			return string.Format("| 0x{0:X4} |  0x{1:X8}  |  0x{2:X8}  |  0x{3:x8}  # {4}", opcode, ctor, callHandler, handler, handler != 0x0 ? name : "Fake Opcode");
		}

		public override string getTableHeader()
		{
			StringBuilder tableHeader = new StringBuilder();
            tableHeader.Append("-------------------------SMSG--------------------------");
            tableHeader.AppendLine();
			tableHeader.Append("+--------+--------------+--------------+--------------+");
			tableHeader.AppendLine ();
			tableHeader.Append("| Opcode |     ctor     |  callHandler |   handler    |");
			tableHeader.AppendLine ();
			tableHeader.Append("+--------+--------------+--------------+--------------+");
			return tableHeader.ToString ();
		}
    }
}
