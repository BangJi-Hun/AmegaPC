using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Threading;     // Thread 클래스 사용을 위해서 추가

namespace Atmega128
{
    public partial class Form1 : Form
    {
        bool conn = false;
        int time = 0;
        byte[] res = new byte[10];

        public Form1()
        {
            InitializeComponent();
            
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            serialPort1.Open();
            CheckForIllegalCrossThreadCalls = false;
        }

        private bool check_sum(byte[] data, int len, byte check_sum)
        {
            byte sum = 0x01;
            for (int i = 0; i < len; i++)
            {
                sum ^= data[i];
            }
            if (sum == 0x02 || sum == 0x03)
            {
                sum = 0x04;
            }

            if (check_sum == sum)
            {
                return true;
            }
            return false;
        }

        private void show_time(byte[] time)
        {
            string t = Encoding.Default.GetString(time, 0, 2) + " : " +
                       Encoding.Default.GetString(time, 2, 2) + " : " +
                       Encoding.Default.GetString(time, 4, 2);
            labelTime.Text = t;
        }

        private void show_vol(byte[] vol)
        {
            string v = Encoding.Default.GetString(vol, 0, 2);
            progressBar1.Value = Int32.Parse(v);
            labelVol.Text = v + "%";
        }

        private void show_button(byte[] button)
        {
            int but = Int32.Parse(Encoding.Default.GetString(button, 0, 1));

            switch (but)
            {
                case 3:
                    if (rmBtn4.BackColor == Color.Red)
                    {
                        rmBtn4.BackColor = Color.Black;
                    }
                    else rmBtn4.BackColor = Color.Red;
                    break;
                case 4:
                    if (rmBtn5.BackColor == Color.Red)
                    {
                        rmBtn5.BackColor = Color.Black;
                    }
                    else rmBtn5.BackColor = Color.Red;
                    break;
            }
        }

        private void serialPort1_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            byte[] recvbuf = new byte[1024];
            make_ack();
            serialPort1.Read(recvbuf, 0, 1);
            if (recvbuf[0] != 0x02)
            {
                while (recvbuf[0] != 0x02)
                {
                    serialPort1.Read(recvbuf, 0, 1);
                }
            }
            serialPort1.Read(recvbuf, 1, 1);
            if (recvbuf[1] != '1')
            {
                make_nak();
                send_res();
                return;
            }
            serialPort1.Read(recvbuf, 2, 1);
            if (recvbuf[2] != '0')
            {
                make_nak();
                send_res();
                return;
            }
            serialPort1.Read(recvbuf, 3, 1);
            serialPort1.Read(recvbuf, 4, 1);
            byte[] data = new byte[50];
            int i;
            int strlen;
            conn = true;

            int commend = Int32.Parse(Encoding.Default.GetString(recvbuf, 3, 2));
            switch (commend)
            {
                case 11:
                    return;
                case 12:
                    return;
                case 13:
                    conn = true;
                    conBtn.BackColor = Color.Blue;
                    time = 0;
                    break;
                case 14:
                    serialPort1.Read(recvbuf, 5, 1);
                    serialPort1.Read(recvbuf, 6, 1);
                    strlen = Int32.Parse(Encoding.Default.GetString(recvbuf, 5, 2));
                    for (i = 0; i < strlen; i++)
                    {
                        serialPort1.Read(recvbuf, 7 + i, 1);
                        data[i] = recvbuf[7 + i];
                        //ack
                    }
                    serialPort1.Read(recvbuf, 7 + i, 1);
                    if (true == check_sum(data, strlen, recvbuf[7 + i]))
                    {
                        show_time(data);
                    }
                    else
                    {
                        make_nak();
                    }
                    break;

                case 15:
                    serialPort1.Read(recvbuf, 5, 1);
                    serialPort1.Read(recvbuf, 6, 1);
                    strlen = Int32.Parse(Encoding.Default.GetString(recvbuf, 5, 2));
                    for (i = 0; i < strlen; i++)
                    {
                        serialPort1.Read(recvbuf, 7 + i, 1);
                        data[i] = recvbuf[7 + i];
                        //ack
                    }
                    serialPort1.Read(recvbuf, 7 + i, 1);
                    if (true == check_sum(data, strlen, recvbuf[7 + i]))
                    {
                        show_vol(data);
                    }
                    else
                    {
                        make_nak();
                    }
                    break;
                case 16:
                    serialPort1.Read(recvbuf, 5, 1);
                    serialPort1.Read(recvbuf, 6, 1);
                    strlen = Int32.Parse(Encoding.Default.GetString(recvbuf, 5, 2));
                    for (i = 0; i < strlen; i++)
                    {
                        serialPort1.Read(recvbuf, 7 + i, 1);
                        data[i] = recvbuf[7 + i];
                    }
                    serialPort1.Read(recvbuf, 7 + i, 1);
                    if (true == check_sum(data, strlen, recvbuf[7 + i]))
                    {
                        show_button(data);
                    }
                    else
                    {
                        make_nak();
                    }
                    break;

            }
            send_res();
            //Thread.Sleep(10);
        }
        private void make_nak()
        {
            byte[] nak = { 0x02, (byte)'0', (byte)'1', (byte)'1', (byte)'2', (byte)'0', (byte)'0', 0x03 };
            res = nak;
        }

        private void make_ack()
        {
            byte[] ack = { 0x02, (byte)'0', (byte)'1', (byte)'1', (byte)'1', (byte)'0', (byte)'0', 0x03 };
            res = ack;
        }
        private void send_res()
        {
            serialPort1.Write(res, 0, res.Length);
        }

        private void make_packet(int num)
        {
            byte number = Convert.ToByte(num+0x30);
            byte check_sum = 0x01;
            check_sum ^= number;

            if (check_sum == 0x02 || check_sum == 0x03)
            {
                check_sum = 0x04;
            }
            byte[] packet = { 0x02, (byte)'0', (byte)'1', (byte)'1', (byte)'6', (byte)'0', (byte)'1', number, check_sum, 0x03 };
            serialPort1.Write(packet, 0, packet.Length);
        }

        private void rmBtn4_Click(object sender, EventArgs e)
        {
            make_packet(3);
            if (rmBtn4.BackColor == Color.Red)
            {
                rmBtn4.BackColor = Color.Black;
            }
            else rmBtn4.BackColor = Color.Red;
        }

        private void rmBtn5_Click(object sender, EventArgs e)
        {
            make_packet(4);
            if (rmBtn5.BackColor == Color.Red)
            {
                rmBtn5.BackColor = Color.Black;
            }
            else rmBtn5.BackColor = Color.Red;
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            time++;
            if (time > 40)
            {
                conn = false;
                conBtn.BackColor = Color.Red;
            }
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            serialPort1.Close();
        }
    }
}
