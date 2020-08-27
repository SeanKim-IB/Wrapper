using System;
using System.Linq;
using System.Collections.Generic;
using System.Text;

namespace IBScanUlitmateWM.include
{
    public class LogManager
    {
        #region "Member variables and constants"
        private static String APPDIR = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().GetName().CodeBase).ToString() + @"\Log\";
        private static String LOGFILE = @"AppLog.txt";
        private static String RDAPROCESSLOGFILE = @"RDAProcessLog.txt";
        private static String RDAERRORLOGFILE = @"LOG.txt";
        private static long MAX_LOG_FILE_SIZE = 50 * 1024;
        #endregion

        /// <summary>
        /// Empty Constructor
        /// </summary>
        public LogManager()
        {
        }

        /// <summary>
        /// Write messages to application log
        /// </summary>
        /// <param name="msg"></param>
        /// <returns></returns>
        public static void WriteToLog(String msg)
        {
            try
            {
                System.IO.DirectoryInfo dirInfo = new System.IO.DirectoryInfo(APPDIR);
                if (!dirInfo.Exists)
                    dirInfo.Create();

                System.IO.FileInfo fileInfo = new System.IO.FileInfo(APPDIR + LOGFILE);
                if (!fileInfo.Exists)
                    fileInfo.Create();

                System.IO.StreamWriter sw = new System.IO.StreamWriter((APPDIR + LOGFILE), true);

                sw.WriteLine("Date & Time:" + DateTime.Now.ToShortDateString() + " " + DateTime.Now.ToShortTimeString());
                sw.WriteLine("~~~~~~~~~~~~~~~~~~~~~~~~~~");
                sw.WriteLine(msg);

                sw.Flush();

                sw.Close();

                sw = null;
            }
            catch
            {
                //System.Windows.Forms.MessageBox.Show("Error", ex.Message);
            }
        }


        /// <summary>
        /// Used to clear the log file contents
        /// </summary>
        public static void ClearLog()
        {
            try
            {
                System.Windows.Forms.DialogResult dlgResult;
                dlgResult = System.Windows.Forms.MessageBox.Show("Are you sure want to clear Log?", "Log Manager", System.Windows.Forms.MessageBoxButtons.YesNo, System.Windows.Forms.MessageBoxIcon.Question, System.Windows.Forms.MessageBoxDefaultButton.Button1);

                switch (dlgResult)
                {
                    case System.Windows.Forms.DialogResult.Yes:
                        //Clear the log file
                        System.IO.DirectoryInfo dirInfo = new System.IO.DirectoryInfo(APPDIR);
                        if (!dirInfo.Exists)
                            dirInfo.Create();

                        System.IO.FileInfo fileInfo = new System.IO.FileInfo(APPDIR + LOGFILE);
                        if (fileInfo.Exists)
                            fileInfo.Delete();
                        fileInfo.Create();

                        fileInfo = new System.IO.FileInfo(APPDIR + RDAERRORLOGFILE);
                        if (fileInfo.Exists)
                            fileInfo.Delete();
                        fileInfo.Create();

                        fileInfo = new System.IO.FileInfo(APPDIR + RDAPROCESSLOGFILE);
                        if (fileInfo.Exists)
                            fileInfo.Delete();
                        fileInfo.Create();
                        break;
                }
            }
            catch
            {
            }
        }


        /// <summary>
        /// Used to get log file content
        /// </summary>
        /// <returns></returns>
        public static String GetLogContent()
        {
            String logContent = String.Empty;
            try
            {
                System.IO.FileInfo fileInfo = new System.IO.FileInfo(APPDIR + LOGFILE);
                if (fileInfo.Exists)
                {
                    System.IO.StreamReader sr = new System.IO.StreamReader((APPDIR + LOGFILE), true);
                    logContent = sr.ReadToEnd();
                    sr.Close();
                    sr = null;
                }
            }
            catch { }
            return logContent;
        }


        /// <summary>
        /// Used to get log file content
        /// </summary>
        /// <returns></returns>
        public static String GetRDALogContent()
        {
            String logContent = String.Empty;
            try
            {
                System.IO.FileInfo fileInfo = new System.IO.FileInfo(APPDIR + RDAPROCESSLOGFILE);

                if (fileInfo.Exists)
                {
                    logContent = "RDA Process Log:" + "\r\n";
                    System.IO.StreamReader sr = new System.IO.StreamReader((APPDIR + RDAPROCESSLOGFILE), true);
                    logContent += sr.ReadToEnd();
                    sr.Close();
                    sr = null;
                }

                fileInfo = new System.IO.FileInfo(APPDIR + RDAERRORLOGFILE);
                if (fileInfo.Exists)
                {
                    logContent += "\r\nRDA Error Log:" + "\r\n";
                    System.IO.StreamReader sr = new System.IO.StreamReader((APPDIR + RDAERRORLOGFILE), true);
                    logContent += sr.ReadToEnd();
                    sr.Close();
                    sr = null;
                }
            }
            catch { }
            return logContent;
        }



        /// <summary>
        /// Used to check log file size if size exceeds 50MB
        /// </summary>
        /// <returns></returns>
        public static bool CheckFileSize()
        {
            bool isDeleted = false;
            System.IO.FileInfo fileSizeInfo = new System.IO.FileInfo(APPDIR + LOGFILE);
            try
            {
                if (fileSizeInfo.Exists)
                {
                    if (fileSizeInfo.Length > MAX_LOG_FILE_SIZE)
                    {
                        fileSizeInfo.Delete();
                        fileSizeInfo.Create();
                        isDeleted = true;
                    }
                }

            }
            catch
            {


            }
            return isDeleted;
        }
    }
}
