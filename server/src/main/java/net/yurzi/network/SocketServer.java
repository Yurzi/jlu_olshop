package net.yurzi.network;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.BufferUnderflowException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Set;

import net.yurzi.common.Handler;
import net.yurzi.common.SocketHandler;
import net.yurzi.controller.SocketController;
import net.yurzi.data.User;
import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.LogManager;
import net.yurzi.data.SnowFlakeGenerateIdWorker;

//对于Nio的Server的封装，用于接收套接字
public class SocketServer implements Runnable {
    private final InetSocketAddress listeningAddress;   //监听的ip地址
    private ServerSocketChannel serverSocket;   //SocketServer
    private SocketController parent;    //控制者
    private Selector selector;  //选择器
    private boolean isRunnable = true;
    private static final Logger logger = LogManager.getLogger(SocketServer.class);
    private static final SnowFlakeGenerateIdWorker idGenerate = new SnowFlakeGenerateIdWorker(0L, 0L);

    private Hashtable<String, Socketer> socketList = new Hashtable<String, Socketer>();
    private Hashtable<SelectionKey, String> ketToIdList = new Hashtable<SelectionKey, String>();


    //初始化链接
    public SocketServer(String host_address, int host_port, SocketController _parent) {
        listeningAddress = new InetSocketAddress(host_address, host_port);
        logger.info("初始化ServerSocket监听地址为:" + listeningAddress.toString());
        parent = _parent;
    }

    public SocketServer(int host_port, SocketController _parent) {
        listeningAddress = new InetSocketAddress("127.0.0.1", host_port);
        logger.info("初始化ServerSocket监听地址为:" + listeningAddress.toString());
        parent = _parent;
    }

    //绑定监听
    public byte listening() {
        byte rt = 0;  //错误状态码
        try {
            logger.info("启动ServerSocket监听于:" + listeningAddress.toString());
            serverSocket = ServerSocketChannel.open();  //获取通道
            serverSocket.bind(listeningAddress);    //绑定通道
            serverSocket.configureBlocking(false);

            logger.info("初始化多路复用器");
            selector = Selector.open();  //打开选择器
            serverSocket.register(selector, SelectionKey.OP_ACCEPT);    //注册读事件
        } catch (IOException e) {
            rt = -1;  //若失败
            logger.error("ServerSocket监听启动失败");
            e.printStackTrace();
        }
        logger.info("ServerSocket监听启动成功");
        return rt;
    }

    @Override   //执行运行函数，将对于的
    public void run() {
        try {
            while (isRunnable) {

                selector.select();  //等待信号
                if (!isRunnable) break;
                logger.info("收到连接池事件");
                //获取可用信号集合并获得迭代器
                Set<SelectionKey> selectionKeys = selector.selectedKeys();
                Iterator<SelectionKey> iterator = selectionKeys.iterator();
                //遍历事件列表
                while (iterator.hasNext()) {
                    //获取事件
                    SelectionKey selectionKey = iterator.next();
                    //如果是连接事件
                    if (selectionKey.isAcceptable()) {
                        //创建连接
                        SocketChannel accept = serverSocket.accept();
                        logger.info("获得新的客户端连接");
                        accept.configureBlocking(false);    //设为非阻塞
                        //注册可读事件
                        SelectionKey key = accept.register(selector, SelectionKey.OP_READ);
                        //分配id，并将此连接加入连接池
                        String Id = idGenerate.generateNextId();
                        logger.info("将Session Id为:" + Id + "的连接加入连接池");
                        ketToIdList.put(key, Id);
                        socketList.put(Id, new Socketer(Id, key, accept, this));
                        SocketHandler handler=(SocketHandler) parent.dispatch("SessionId", Id);
                        if(handler!=null)parent.submit(handler);
                    } else if (selectionKey.isReadable()) {
                        //读事件
                        String id = ketToIdList.get(selectionKey);
                        logger.info("接收到客户端:" + id + "的消息");
                        try {
                            Socketer socketer=socketList.get(id);
                            if (socketer==null){
                                selectionKey.cancel();
                                rmSocketById(id);
                                continue;
                            }
                            String content = socketer.read();   //将消息读出
                            if (content != null) {
                                SocketHandler handler=(SocketHandler) parent.dispatch(content);
                                parent.submit(handler);
                            }
                        } catch (IOException | BufferUnderflowException e) {
                            logger.warn("注销无用的channel");
                            selectionKey.cancel();  //注销无用的键
                            //e.printStackTrace();
                        }
                    }
                    iterator.remove();
                }
                selector.selectedKeys().clear();    //清空多路复用器中的事件
            }
        } catch (IOException e) {
            if (isRunnable) {
                logger.error("多路复用器异常");
                e.printStackTrace();
            }
        }
    }

    //Getter&Setter
    public InetSocketAddress getListeningAddress() {
        return listeningAddress;
    }

    public boolean getRunnable() {
        return isRunnable;
    }

    public void setRunnable(boolean to) {
        isRunnable = to;
    }

    public Socketer getSocketById(String id) {
        return socketList.get(id);
    }

    public Hashtable<String,Socketer> getSocketList(){
        return socketList;
    }


    public void rmSocketById(String id) {
        SelectionKey key = socketList.get(id).get_key();
        ketToIdList.remove(key);
        socketList.remove(id);
        parent.unmapSocketAndUser(id);
    }

    public void exit() {
        try {
            logger.info("关闭多路复用器");
            selector.wakeup();
            selector.close();
        } catch (IOException e) {
            logger.error("多路复用器关闭异常");
            e.printStackTrace();
        }
    }
}
