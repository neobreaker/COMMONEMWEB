using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace YHServer.YHLib
{

    struct YHElement
    {
        public byte[] m_data;
        public int m_len;

        public YHElement(byte[] data, int len)
        {
            if (data != null)
            {
                m_data = new byte[4096];
                Array.Copy(data, m_data, len);
            }
            else
            {
                m_data = null;
            }
            m_len = len;
        }
    }

    class YHbuffer
    {
        private int m_blocks;
        private Queue<YHElement> m_data_queue;

        public YHbuffer(int blocks)
        {
            m_blocks = blocks;

            m_data_queue = new Queue<YHElement>(m_blocks);

        }

        public void Enqueue(byte[] data, int len)
        {
            YHElement e = new YHElement(data, len);
            
            if (m_data_queue.Count == m_blocks)
                m_data_queue.Dequeue();
            m_data_queue.Enqueue(e);
        }

        public YHElement Dequeue()
        {
            YHElement e = new YHElement(null, 0);

            if (m_data_queue.Count != 0)
                return m_data_queue.Dequeue();
            return e;
        }

        public void Clear()
        {
            m_data_queue.Clear();
        }
    }
}
;