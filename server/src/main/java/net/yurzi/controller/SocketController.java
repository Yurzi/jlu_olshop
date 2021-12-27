package net.yurzi.controller;
/**
 * Socket控制器用于管理socket
 */

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import net.yurzi.common.*;
import net.yurzi.data.DataPackage;
import net.yurzi.data.User;
import net.yurzi.network.SocketServer;
import net.yurzi.network.Socketer;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.lang.reflect.Type;
import java.util.Hashtable;
import java.util.Map;
import java.util.function.Function;

public class SocketController implements Runnable {
    private final static Logger logger = LogManager.getLogger(SocketController.class);
    private int m_port;   //该控制器管理的Server端口
    private SocketServer m_socketServer;
    private Controller parent;  //上级控制器
    private static Hashtable<String, Function<String, SocketHandler>> m_handlerMap = new Hashtable<String, Function<String, SocketHandler>>();
    private Hashtable<String, Integer> socket2UserList=new Hashtable<String,Integer>();   //用户和socket的映射表
    private Thread m_thread;

    public SocketController(int port, Controller _parent) {
        parent = _parent;
        logger.info("初始化网络模块ing...");
        m_port = port;
        logger.info("构建网络模块表");
        this.init();
        m_socketServer = new SocketServer(m_port, this);
        m_socketServer.setRunnable(true);
        logger.info("启动监听...");
        int try_count = 0;
        while (m_socketServer.listening() == -1 && try_count < 3) {
            logger.warn("尝试重新启动监听...");
            ++try_count;
        }
        if (try_count == 3) {
            logger.error("网络模块启动失败");
            m_socketServer.setRunnable(false);  //关闭网络模块
        }
    }

    //初始化Handler列表
    private void init() {
        logger.info("加入Handler:SessionId");
        m_handlerMap.put("SessionId", (String rawData) -> new SessionIdHandler(rawData, this));
        m_handlerMap.put("UserLogin",(String rawData)->new UserLoginHandler(rawData,this));
        m_handlerMap.put("UserLogout",(String rawData)->new UserLogoutHandler(rawData,this));
        m_handlerMap.put("UserRegister",(String rawData)->new UserRegisterHandler(rawData,this));
        m_handlerMap.put("ItemList",(String rawData)->new ItemListHandler(rawData,this));
        m_handlerMap.put("Buy",(String rawData)->new BuyHandler(rawData,this));
        m_handlerMap.put("UserDel",(String rawData)->new UserDelHandler(rawData,this));
        m_handlerMap.put("UserUpdate",(String rawData)->new UserUpdateHandler(rawData,this));
        m_handlerMap.put("ItemDel",(String rawData)->new ItemDelHandler(rawData,this));
        m_handlerMap.put("ItemUpdate",(String rawData)->new ItemUpdateHandler(rawData,this));
    }

    public Handler dispatch(String rawData) {
        Gson gson = new Gson();
        DataPackage<String> tmp = gson.fromJson(rawData, new TypeToken<DataPackage<String>>(){}.getType());
        return dispatch(tmp.handlerName, rawData);
    }

    public Handler dispatch(String handlerName, String rawData) {
        if (m_handlerMap.get(handlerName)!=null) {
            return m_handlerMap.get(handlerName).apply(rawData);
        }
        //todo:加入上级的任务派发
        return null;
    }

    public void submit(SocketHandler handler) {
        if (handler==null){
            logger.error("提交了一个空的Handler对象");
            return;
        }
        parent.submit(handler); //将任务提交至上级控制器
    }

    //根据session_id来获取socket
    public Socketer getSocketById(String id) {
        return m_socketServer.getSocketById(id);
    }
    public Hashtable<String,Socketer> getSocketList(){
        return m_socketServer.getSocketList();
    }

    @Override
    public void run() {
        m_socketServer.run();
    }

    public void exit() {
        m_socketServer.setRunnable(false);
        logger.info("关闭SocketServer....");
        m_socketServer.exit();
    }


    public void mapSocketToUser(String session,User user){
        logger.info("链接Session:"+session+"->User:"+user.getId());
        socket2UserList.put(session,user.getId());
        parent.addLoginUser(user.getId(),user);
    }

    public String getMappedSessionByUser(int id){
        for (Map.Entry<String,Integer> entry:socket2UserList.entrySet()) {
            if (entry.getValue()==id){
                return entry.getKey();
            }
        }
        return null;
    }

    public User getMappedUser(String session){
        return parent.getLoginUser(socket2UserList.get(session));
    }

    public void unmapSocketAndUser(String session){
        if (session==null)return;
        if (socket2UserList.get(session)==null){
            return;
        }
        int id=socket2UserList.get(session);
        socket2UserList.remove(session);
        parent.rmLoginUser(id);
    }

    public Controller getParent(){
        return parent;
    }
}
