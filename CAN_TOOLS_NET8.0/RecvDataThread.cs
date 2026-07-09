using System;
using System.Runtime.InteropServices;
using System.Threading;
using ZLGCAN;

namespace CAN_TOOLS
{
    // CAN/CAN FD receive worker. One instance owns one background thread.
    class RecvDataThread
    {
        public delegate void RecvCANDataEventHandler(ZCAN_Receive_Data[] data, uint len, uint ch);
        public delegate void RecvFDDataEventHandler(ZCAN_ReceiveFD_Data[] data, uint len, uint ch);

        private const int TYPE_CAN = 0;
        private const int TYPE_CANFD = 1;
        private const uint MAX_RECEIVE_BATCH = 2500;

        private volatile bool m_bStart;
        private IntPtr channel_handle_;
        private IntPtr channel_handle2_;
        private Thread? recv_thread_;
        private readonly object stateLock = new object();

        public event RecvCANDataEventHandler? RecvCANData;
        public event RecvFDDataEventHandler? RecvFDData;

        public void SetStart(bool start)
        {
            Thread? threadToJoin = null;

            lock (stateLock)
            {
                if (start)
                {
                    if (recv_thread_?.IsAlive == true)
                        return;

                    m_bStart = true;
                    recv_thread_ = new Thread(RecvDataFunc)
                    {
                        IsBackground = true,
                        Name = "CAN receive thread"
                    };
                    recv_thread_.Start();
                    return;
                }

                m_bStart = false;
                threadToJoin = recv_thread_;
                recv_thread_ = null;
            }

            if (threadToJoin != null && threadToJoin != Thread.CurrentThread)
            {
                threadToJoin.Join();
            }
        }

        public void SetChannelHandle(IntPtr channel_handle, IntPtr channel_handle2)
        {
            lock (stateLock)
            {
                channel_handle_ = channel_handle;
                channel_handle2_ = channel_handle2;
            }
        }

        private void RecvDataFunc()
        {
            while (m_bStart)
            {
                IntPtr channel1;
                IntPtr channel2;
                lock (stateLock)
                {
                    channel1 = channel_handle_;
                    channel2 = channel_handle2_;
                }

                try
                {
                    ReceiveCan(channel1, 0);
                    ReceiveCanFd(channel1, 0);
                    ReceiveCan(channel2, 1);
                    ReceiveCanFd(channel2, 1);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"CAN接收线程异常: {ex.Message}");
                }

                Thread.Sleep(30);
            }
        }

        private void ReceiveCan(IntPtr channelHandle, uint channel)
        {
            if (channelHandle == IntPtr.Zero) return;

            uint pending = Method.ZCAN_GetReceiveNum(channelHandle, TYPE_CAN);
            uint requested = Math.Min(pending, MAX_RECEIVE_BATCH);
            if (requested == 0) return;

            int itemSize = Marshal.SizeOf<ZCAN_Receive_Data>();
            IntPtr ptr = Marshal.AllocHGlobal(checked((int)requested * itemSize));
            try
            {
                uint received = Method.ZCAN_Receive(channelHandle, ptr, requested, 50);
                received = Math.Min(received, requested);
                if (received == 0) return;

                var frames = new ZCAN_Receive_Data[received];
                for (int i = 0; i < received; i++)
                {
                    frames[i] = Marshal.PtrToStructure<ZCAN_Receive_Data>(IntPtr.Add(ptr, i * itemSize));
                }

                RecvCANData?.Invoke(frames, received, channel);
            }
            finally
            {
                Marshal.FreeHGlobal(ptr);
            }
        }

        private void ReceiveCanFd(IntPtr channelHandle, uint channel)
        {
            if (channelHandle == IntPtr.Zero) return;

            uint pending = Method.ZCAN_GetReceiveNum(channelHandle, TYPE_CANFD);
            uint requested = Math.Min(pending, MAX_RECEIVE_BATCH);
            if (requested == 0) return;

            int itemSize = Marshal.SizeOf<ZCAN_ReceiveFD_Data>();
            IntPtr ptr = Marshal.AllocHGlobal(checked((int)requested * itemSize));
            try
            {
                uint received = Method.ZCAN_ReceiveFD(channelHandle, ptr, requested, 50);
                received = Math.Min(received, requested);
                if (received == 0) return;

                var frames = new ZCAN_ReceiveFD_Data[received];
                for (int i = 0; i < received; i++)
                {
                    frames[i] = Marshal.PtrToStructure<ZCAN_ReceiveFD_Data>(IntPtr.Add(ptr, i * itemSize));
                }

                RecvFDData?.Invoke(frames, received, channel);
            }
            finally
            {
                Marshal.FreeHGlobal(ptr);
            }
        }
    }
}
