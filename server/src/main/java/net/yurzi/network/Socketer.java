package net.yurzi.network;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.EOFException;
import java.io.IOException;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.SocketChannel;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.locks.ReentrantLock;

public class Socketer {
    private final static Logger logger = LogManager.getLogger(Socketer.class);
    private String m_session_id;    //id
    private SocketServer m_parent;    //控制者
    private SocketChannel m_socket; //套接字的对象
    private SelectionKey m_key; //对应的key
    private ReentrantLock readLock=new ReentrantLock(true);
    private ReentrantLock wirteLock=new ReentrantLock();

    public Socketer(String session_id, SelectionKey key, SocketChannel socket, SocketServer parent) {
        //初始化Socket
        m_session_id = session_id;
        m_key = key;
        m_socket = socket;
        m_parent = parent;
    }

    //对读的封装
    public String read() throws IOException {
        logger.info("初始化接收缓冲区");
        ByteBuffer buf = ByteBuffer.allocate(4);  //读取数据包长度
        readLock.lock();
        int bytesRead = m_socket.read(buf);
        buf.flip();
        int length=-1;
        try {
            length = buf.getInt();
        }catch (BufferUnderflowException e){
                logger.warn("远程连接已断开！");
                logger.warn("清除Session:"+m_session_id);
                readLock.unlock();
                m_socket.close();
                m_parent.rmSocketById(m_session_id);
                throw e;
        }
        readLock.unlock();
        logger.info("获取数据包长度为:" + length);
        if(length==-1)return null;
        buf = ByteBuffer.allocate(length);    //准备一定长度的缓冲区
        logger.info("接收数据包...");
        readLock.lock();
        while (buf.hasRemaining()) {
            try {
                m_socket.read(buf); //读取数据包内容
            } catch (IOException e) {
                logger.error("数据接受异常Session:" + m_session_id);
                logger.warn("清除Session:"+m_session_id);
                readLock.unlock();
                m_socket.close();
                m_parent.rmSocketById(m_session_id);
                throw e;
            }
        }
        readLock.unlock();
        logger.info("接收完成");
        buf.flip(); //切换缓冲区模式
        //输出结果
        CharsetDecoder decoder = StandardCharsets.UTF_8.newDecoder();
        return decoder.decode(buf).toString();
    }

    //对写的封装
    public void write(String str) throws IOException {
        logger.info("初始化发送数据包");

        ByteBuffer buf = ByteBuffer.allocate(str.getBytes(StandardCharsets.UTF_8).length + Integer.BYTES);     //申请缓冲区
        logger.info("缓冲区申请成功");
        buf.putInt(str.getBytes(StandardCharsets.UTF_8).length);    //加入数据包长度
        logger.info("数据包长度为:"+(str.getBytes(StandardCharsets.UTF_8).length+Integer.BYTES));
        buf.put(str.getBytes(StandardCharsets.UTF_8));  //将字符串内容放入缓冲区
        logger.info("发送数据为:"+str);
        buf.flip(); //缓冲区反转
        //buf.rewind();
        //logger.debug("发送内容为:"+str.length()+str);

        logger.info("发送数据包...");
        wirteLock.lock();
        try {
            while (buf.hasRemaining()) {
                m_socket.write(buf);    //进行写
            }
        }catch (IOException e){
            wirteLock.unlock(); //解锁线程锁
            logger.error("数据发送异常,Session:"+m_session_id);
            logger.warn("清除Session:"+m_session_id);
            readLock.unlock();
            m_socket.close();
            m_parent.rmSocketById(m_session_id);
        }
        wirteLock.unlock();
        logger.info("发送完成");
    }

    //Getter
    public String get_session_id() {
        return m_session_id;
    }

    public SocketServer get_parent() {
        return m_parent;
    }

    public SocketChannel get_socket() {
        return m_socket;
    }

    public SelectionKey get_key() {
        return m_key;
    }

    protected void finalize() {
        try {
            if (m_socket.isOpen()) m_socket.close();   //关闭套接字
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
