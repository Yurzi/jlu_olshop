package net.yurzi.controller;

import net.yurzi.common.Handler;
import net.yurzi.common.MybatisUitls;
import net.yurzi.dao.ItemMapper;
import net.yurzi.dao.TableInit;
import net.yurzi.data.Item;
import net.yurzi.data.Privilege;
import net.yurzi.data.User;
import org.apache.ibatis.session.SqlSession;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.math.BigDecimal;
import java.util.*;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Controller implements Runnable {
    private final static Logger logger = LogManager.getLogger(Controller.class);
    private ExecutorService executorService;    //线程池
    private SocketController socketController;  //网络模块控制器
    private Queue<Handler> taskList;        //任务缓冲列表
    private boolean isRunable = true;       //是否可运行
    private Thread m_socketServer_thread;   //socket进程
    private Hashtable<Integer,User> login_UserList=new Hashtable<Integer,User>(); //已经登录的用户的列表

    public Controller() {
        logger.info("加载数据库...");
        initTable();
        int proc_count = Runtime.getRuntime().availableProcessors();
        logger.info("当前设备处理器数量为:" + proc_count + "将使用" + proc_count / 2 + "个核心");
        logger.info("初始化线程池...");
        executorService = Executors.newFixedThreadPool(proc_count / 2); //初始化线程池
        taskList = new LinkedList<Handler>();
        logger.info("初始化网络模块...");
        socketController = new SocketController(11451, this);
    }

    public void initTable(){
        SqlSession sqlSession=MybatisUitls.getSqlSession();
        logger.info("检查表user_login...");
        TableInit tableInit=sqlSession.getMapper(TableInit.class);
        tableInit.initUserTable();  //初始化用户表
        User rootUser=tableInit.getRootUser(Privilege.root);
        if (rootUser==null||rootUser.getNickname()==null){
            logger.warn("系统管理员不存在！初始化管理员:Yurzi,初始密码为:yurzi");
            rootUser=new User("Yurzi","yurzi",Privilege.root, new BigDecimal("0"),"");
            tableInit.initRootUser(rootUser);
            logger.debug("向数据库提交事务");
            sqlSession.commit();    //提交
        }
        logger.info("检查表item_list...");
        tableInit.initItemListTable();  //初始化商品表

        sqlSession.close();     //关闭数据库连接
    }

    public void submit(Handler handler) {
        if(handler == null){
            logger.error("提交了一个空的Handler对象");
            return;
        }
        logger.info("提交了一个任务");
        taskList.add(handler);
        synchronized (this) {
            this.notify();
        }
    }

    public void exit() {
        logger.info("关闭网络模块....");
        socketController.exit();
        logger.info("关闭线程池....");
        executorService.shutdown();
        synchronized (this) {
            this.notify();
        }
    }

    @Override
    public void run() {
        logger.info("启动网络模块....");
        m_socketServer_thread = new Thread(socketController);
        m_socketServer_thread.setName("Network");
        m_socketServer_thread.start();
        logger.info("网络模块启动成功");
        synchronized (this) {
            while (isRunable) {
                if (!taskList.isEmpty()) {
                    logger.info("将任务提交至线程池");
                    executorService.submit(taskList.remove());
                } else {
                    try {
                        this.wait();
                    } catch (InterruptedException e) {
                        if (this.isRunable) {
                            logger.error("控制器线程异常");
                            e.printStackTrace();
                        }
                    }
                }
            }
        }
    }

    public void setRunable(boolean runable) {
        isRunable = runable;
    }

    public boolean getRunable() {
        return isRunable;
    }

    public void addLoginUser(int id,User user){
        login_UserList.put(id,user);
    }

    public void rmLoginUser(int id){
        login_UserList.remove(id);
    }

    public User getLoginUser(int id){
        logger.info("查找Id为:"+id+"的用户");
        return login_UserList.getOrDefault(id, null);

    }
}
