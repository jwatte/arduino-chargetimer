using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GenerateSerial
{
    public partial class AVRemoteExplorerForm : Form
    {
        public AVRemoteExplorerForm()
        {
            InitializeComponent();
            serialPortAVR.Open();
            serialPortAVR.DataReceived += new System.IO.Ports.SerialDataReceivedEventHandler(serialPort1_DataReceived);
            Application.Idle += new EventHandler(Application_Idle);
        }

        int oldbtr = -1;
        int oldbtw = -1;

        void Application_Idle(object sender, EventArgs e)
        {
            int btr = serialPortAVR.BytesToRead;
            if (btr != oldbtr)
            {
                labelToRead.Text = btr.ToString();
            }
            int btw = serialPortAVR.BytesToWrite;
            if (btw != oldbtw)
            {
                labelToWrite.Text = btw.ToString();
            }
            if (btr > 0)
            {
                DoRead();
            }
            lock (ibuf)
            {
                int n = ibuf.Count;
                if (n > 0 && ibuf[0] <= n)
                {
                    //  decode a command
                    if (ibuf[0] == 0)
                    {
                        //  null command
                        ibuf.RemoveRange(0, 1);
                        listBoxInfo.Items.Add("-> null cmd");
                    }
                    else
                    {
                        sbyte[] payload = null;
                        if (ibuf[0] > 2)
                        {
                            payload = new sbyte[ibuf[0] - 2];
                            ibuf.CopyTo(2, payload, 0, ibuf[0] - 2);
                        }
                        this.on_response(ibuf[1], payload);
                        ibuf.RemoveRange(0, ibuf[0]);
                    }
                }
            }
        }

        void serialPort1_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            DoRead();
        }

        void DoRead()
        {
            lock (ibuf)
            {
                int btr = serialPortAVR.BytesToRead;
                byte[] buf = new byte[btr];
                serialPortAVR.Read(buf, 0, btr);
                //  this is terrible!
                foreach (byte b in buf)
                {
                    ibuf.Add((sbyte)b);
                }
            }
        }

        List<sbyte> ibuf = new List<sbyte>();

        private void timer1_Tick(object sender, EventArgs e)
        {
            Application_Idle(sender, e);
        }

        public void on_response(sbyte cmd, sbyte[] data)
        {
            switch (cmd)
            {
                case 1:
                    on_error(data);
                    break;
                case 2:
                    on_status(data);
                    break;
                default:
                    on_unknown(cmd, data);
                    break;
            }
        }

        public unsafe static string stringFromData(sbyte[] data)
        {
            if (data == null)
            {
                return "";
            }
            fixed (sbyte *sp = data)
            {
                return new String(sp, 0, data.Length);
            }
        }

        public static string hexdump(sbyte[] data)
        {
            StringBuilder sf = new StringBuilder();
            for (int i = 0, n = data.Length; i != n; ++i)
            {
                sf.AppendFormat("{0:X2} ", data[i]);
            }
            return sf.ToString();
        }

        public void on_error(sbyte[] data)
        {
            listBoxInfo.Items.Add(String.Format("-> error('{0}')", stringFromData(data)));
        }

        public void on_status(sbyte[] data)
        {
            listBoxInfo.Items.Add(String.Format("-> status('{0}')", stringFromData(data)));
        }

        public void on_unknown(sbyte code, sbyte[] data)
        {
            listBoxInfo.Items.Add(String.Format("-> unknown code {0} ({1})", code, hexdump(data)));
        }

        public void send_cmd(byte cmd, byte[] data)
        {
            byte len = 2;
            if (data != null)
            {
                if (data.Length > 30)
                {
                    throw new InvalidOperationException("Maximum payload size is 30 bytes");
                }
                len = (byte)(len + data.Length);
            }
            serialPortAVR.Write(new byte[] { len, cmd }, 0, 2);
            if (data != null)
            {
                serialPortAVR.Write(data, 0, data.Length);
            }
        }

        public void send_reset()
        {
            send_cmd(0, null);
        }

        public void send_status()
        {
            send_cmd(1, null);
        }

        public void send_carrier(ushort carrier, byte dutycycle)
        {
            send_cmd(2, new byte[] { (byte)(carrier >> 8), (byte)(carrier & 0xff), dutycycle });
        }

        ushort[] codes = new ushort[200];
        int nCodes = 0;
        byte[] codeBytes = new byte[400];

        private void addCode(bool high, ushort us)
        {
            if (nCodes == codes.Length)
            {
                throw new InvalidOperationException("addCode() -- too many codes!");
            }
            //  I only have 8 us precision
            codes[nCodes] = (ushort)(us >> 3);
            if (high)
            {
                codes[nCodes] |= 0x8000;
            }
        }

        private void addBitsNEC(uint data, int count)
        {
            for (int i = 0; i < count; ++i)
            {
                addCode(true, 563);
                if ((data & (1 << i)) != 0)
                {
                    addCode(false, 1125);
                }
                else
                {
                    addCode(false, 563);
                }
            }
        }

        private void send_codes()
        {
            for (int i = 0; i < nCodes; ++i)
            {
                codeBytes[i * 2] = (byte)(codes[i] >> 8);
                codeBytes[i * 2 + 1] = (byte)(codes[i] & 0xffu);
            }
            send_cmd(3, new byte[] { (byte)nCodes });
            //  that command is followed by un-framed code data
            serialPortAVR.Write(codeBytes, 0, nCodes * 2);
            nCodes = 0;
        }

        public void send_nec(ushort target, byte cmd)
        {
            nCodes = 0;

            //  AGC preamble
            addCode(true, 9000);
            addCode(false, 2250);

            //  16 bit address
            addBitsNEC(target, 16);
            //  8 bit command
            addBitsNEC(cmd, 8);
            //  8 bit command, inverted
            addBitsNEC(cmd ^ 0xffu, 8);
            send_codes();
        }

        public static ushort UInt16Parse(String s)
        {
            if (s.Length > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
            {
                return UInt16.Parse(s.Substring(2), System.Globalization.NumberStyles.HexNumber);
            }
            return UInt16.Parse(s);
        }

        public static byte byteParse(String s)
        {
            if (s.Length > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
            {
                return byte.Parse(s.Substring(2), System.Globalization.NumberStyles.HexNumber);
            }
            return byte.Parse(s);
        }

        private void buttonReset_Click(object sender, EventArgs e)
        {
            listBoxInfo.Items.Add("Reset");
            this.send_reset();
        }

        private void buttonStatus_Click(object sender, EventArgs e)
        {
            listBoxInfo.Items.Add("Status");
            this.send_status();
        }

        private void buttonCarrier_Click(object sender, EventArgs e)
        {
            try
            {
                ushort carrier = UInt16Parse(textBoxCarrier.Text);
                byte dutycycle = byteParse(textBoxDutyCycle.Text);
                listBoxInfo.Items.Add(String.Format("carrier {0} dutycycle {1}", carrier, dutycycle));
                this.send_carrier(carrier, dutycycle);
            }
            catch (System.Exception x)
            {
                MessageBox.Show(x.Message);
            }
        }

        private void buttonSendNEC_Click(object sender, EventArgs e)
        {
            try
            {
                ushort target = UInt16Parse(textBoxTargetNEC.Text);
                byte cmd = byteParse(textBoxCmdNEC.Text);
                listBoxInfo.Items.Add(String.Format("NEC target {0} cmd {1}", target, cmd));
                this.send_nec(target, cmd);
            }
            catch (System.Exception x)
            {
                MessageBox.Show(x.Message);
            }
        }
    }
}
