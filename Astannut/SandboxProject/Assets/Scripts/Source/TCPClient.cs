using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using Newtonsoft.Json;

namespace Sandbox
{
    class TcpClient
    {
        private Socket client_socket;
        private byte[] recvData = new byte[0];
        private int length = 0;
        public dpsDamage damge;

        public void Create()
        {
            client_socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

            IPAddress serverIP = IPAddress.Parse("127.0.0.1");
            int serverPort = 5001;

            IPEndPoint serverEndPoint = new IPEndPoint(serverIP, serverPort);

            client_socket.Connect(serverEndPoint);

        }

        public void socket_recive()
        {
            byte[] buffer = new byte[1024];
            int bytesRead = client_socket.Receive(buffer);
            byte[] newData = new byte[recvData.Length + bytesRead];
            Buffer.BlockCopy(recvData, 0, newData, 0, recvData.Length);
            Buffer.BlockCopy(buffer, 0, newData, recvData.Length, bytesRead);
            recvData = newData;
        }

        public void Recive()
        {
            // 使用Poll方法检查是否有数据可读
            if (client_socket.Poll(1660, SelectMode.SelectRead))
            {
                
                if (length > 0)
                {
                    socket_recive();
                    if (recvData.Length >= length + 4)
                    {
                        byte[] dataBytes = new byte[length];
                        Buffer.BlockCopy(recvData, 4, dataBytes, 0, length);
                        byte[] remainingData = new byte[recvData.Length - (length + 4)];
                        Buffer.BlockCopy(recvData, length + 4, remainingData, 0, recvData.Length - (length + 4));
                        recvData = remainingData;
                        if (recvData.Length > 3)
                        {
                            length = BitConverter.ToInt32(recvData, 0);
                        }
                        else
                        {
                            length = 0;
                        }
                        try
                        {
                            string data_str = Encoding.UTF8.GetString(dataBytes);
                            var data = JsonConvert.DeserializeObject<Dictionary<string, object>>(data_str);
                            if (data != null && data.ContainsKey("protoname"))
                            {
                                //WriteToFile(data_str);
                                Dps dps = new Dps();
                                damge = dps.parseProto((string)data["protoname"], data);
                                Console.WriteLine("parseProto over");
                                Console.WriteLine(damge.Value);
                            }
                        }
                        catch (JsonException)
                        {
                            Console.WriteLine("error");
                        }
                    }
                }
                else
                {
                    socket_recive();
                    if (recvData.Length >= 4)
                    {
                        length = BitConverter.ToInt32(recvData, 0);
                    }
                }
            }
        }
        private void WriteToFile(string data)
        {
            // 这里实现将数据写入文件的逻辑，例如：
            // File.WriteAllText("data.txt", data);
            Console.WriteLine(data);
        }

    }
}

