using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data.SQLite;

namespace OpcodeDiffer
{
    public class SQLiteDB
    {
        protected SQLiteConnection dbConnection;

        public SQLiteDB(string dbPath)
        {
            dbConnection = new SQLiteConnection(string.Format("Data Source={0};FailIfMissing=True;", dbPath));
        }
        
        ~SQLiteDB()
        {
            dbConnection.Dispose();
        }

        public bool OpenConnection()
        {
            try
            {
                dbConnection.Open();
            }
            catch(SQLiteException /*e*/)
            {
                return false;
            }

            return true;
        }
    }

    public class BinDiff : SQLiteDB
    {
        public BinDiff(string dbPath) : base(dbPath) { }

        //public Int32 getOldFunction(uint NewFunctionAddr)
        //{
        //    string sql = string.Format("SELECT * FROM function WHERE address1={0}", NewFunctionAddr.ToString());
        //    SQLiteCommand command = new SQLiteCommand(sql, dbConnection);
        //    SQLiteDataReader reader = command.ExecuteReader();
        //    if (reader.Read())
        //        return reader.GetInt32(2);
        //    else
        //        return 0;
        //}

        //public double getCertianty(uint NewFunctionAddr)
        //{
        //    string sql = string.Format("SELECT * FROM function WHERE address1={0}", NewFunctionAddr.ToString());
        //    SQLiteCommand command = new SQLiteCommand(sql, dbConnection);
        //    SQLiteDataReader reader = command.ExecuteReader();
        //    if (reader.Read())
        //        return reader.GetDouble(4);
        //    else
        //        return 0.0f;
        //}
    }

    public class PacketDumpDB : SQLiteDB
    {
        public PacketDumpDB(string dbPath) 
            : base(dbPath)
        {
            getCmsgCaller = new SQLiteCommand("SELECT caller FROM CMSG WHERE ", dbConnection);
            getSmsgHandler = new SQLiteCommand("", dbConnection);
        }

        SQLiteCommand getCmsgCaller;
        SQLiteCommand getSmsgHandler;
    }

    public class NameDB : SQLiteDB
    {
        public NameDB(string dbPath)
            : base(dbPath)
        {
            writeNameCmd = new SQLiteCommand("UPDATE @type set name=@name WHERE opcode=@opcode", dbConnection);
            getNameFromCaller = new SQLiteCommand("SELECT name FROM CMSG WHERE caller=@caller", dbConnection);
            getNameFromHandler = new SQLiteCommand("SELECT name FROM SMSG WHERE handler=@handler", dbConnection);
        }

        SQLiteCommand writeNameCmd;
        SQLiteCommand getNameFromCaller;
        SQLiteCommand getNameFromHandler;

        public void writeNameCMSG(int opcode, string name)
        {
            // Write parameters
            writeNameCmd.Parameters.AddWithValue("@type", "CMSG");
            writeNameCmd.Parameters.AddWithValue("@name", name);
            writeNameCmd.Parameters.AddWithValue("@opcode", opcode);

            // Execute query and clear parameters
            writeNameCmd.ExecuteNonQuery();
            writeNameCmd.Parameters.Clear();
        }

        public void writeNameSMSG(int opcode, string name)
        {
            // Write parameters
            writeNameCmd.Parameters.AddWithValue("@type", "SMSG");
            writeNameCmd.Parameters.AddWithValue("@name", name);
            writeNameCmd.Parameters.AddWithValue("@opcode", opcode);

            // Execute query and clear parameters
            writeNameCmd.ExecuteNonQuery();
            writeNameCmd.Parameters.Clear();
        }

        public string GetNameFromHandlerSMSG(int handler)
        {
            // Add parameter
            getNameFromHandler.Parameters.AddWithValue("@handler", handler);

            // Execute query
            return (string)getNameFromHandler.ExecuteScalar();

        }

        public string GetNameFromCallerCMSG(int caller)
        {
            // Add parameter
            getNameFromCaller.Parameters.AddWithValue("@caller", caller);

            // Execute query
            return (string)getNameFromCaller.ExecuteScalar();
        }
    }
}

