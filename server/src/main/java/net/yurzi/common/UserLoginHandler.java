package net.yurzi.common;

import com.alibaba.fastjson.JSON;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.reflect.TypeToken;
import net.yurzi.controller.SocketController;
import net.yurzi.dao.UserMapper;
import net.yurzi.data.DataPackage;
import net.yurzi.data.User;
import net.yurzi.network.SocketPackgeType;
import net.yurzi.network.Socketer;
import org.apache.ibatis.session.SqlSession;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.IOException;

public class UserLoginHandler extends SocketHandler{

    private static final Logger logger= LogManager.getLogger(UserLoginHandler.class);

    public UserLoginHandler(String rawData, SocketController _parent) {
        super(rawData, _parent);
        m_name="UserLogin";
    }

    public UserLoginHandler(DataPackage data, SocketController _parent) {
        super(data, _parent);
        m_name="UserLogin";
    }


    @Override
    protected void resolveRawData() {
        logger.info("解析数据....");
        //logger.info("解析数据为:"+m_rawData);
        //创建json解析器
        m_data= JSON.parseObject(m_rawData,DataPackage.class);
        logger.info("请求类型为:"+m_data.type);
        //logger.info("内容为:"+m_data.contents.get(0));
    }

    @Override
    public void run() {
        resolveRawData();   //解析数据
        logger.info("数据解析完成。");
        if (m_data.type.equals(SocketPackgeType.request)){
            //如果是request，则代表请求登录
            //查找用户信息
            User login_user=JSON.parseObject((String) m_data.contents.get(0),User.class); //获取欲登录用户信息
            logger.info("处理用户:"+login_user.getNickname());
            //查询数据库
            SqlSession sqlSession = MybatisUitls.getSqlSession();
            //logger.info("打开数据库连接");
            UserMapper userMapper= sqlSession.getMapper(UserMapper.class);
            logger.info("查找用户");
            User user=userMapper.getUserByNickname(login_user.getNickname()); //查找
            sqlSession.close();
            logger.info("关闭数据库事务");
            Gson gson=new GsonBuilder().serializeNulls().create();
            if (user==null){
                logger.info("用户:"+login_user.getNickname()+"不存在");
                //若为空则不存在该用户,向客户端发送无此用户的消息
                DataPackage<String> sendData=new DataPackage<String>(m_data.session_id,m_name, SocketPackgeType.response);
                sendData.contents.add("NOT_FIND");
                //转换数据
                String sendRawData=gson.toJson(sendData,new TypeToken<DataPackage<String>>(){}.getType());
                Socketer socketer=parent.getSocketById(m_data.session_id);

                if(socketer!=null) {
                    try {
                        //发送消息
                        socketer.write(sendRawData);
                    } catch (IOException e) {
                        logger.error("数据发送异常，连接可能已经关闭!");
                    }
                }
            }else {
                //如果不为空则做检查
                if (!login_user.getPassword().equals(user.getPassword())) {
                    logger.info("登录用户的密码错误");
                    DataPackage<String> sendData = new DataPackage<String>(m_data.session_id, m_name, SocketPackgeType.response);
                    sendData.contents.add("NO_RIGHT_PASSWORD");
                    //转换数据
                    String sendRawData = gson.toJson(sendData, new TypeToken<DataPackage<String>>() {
                    }.getType());
                    Socketer socketer = parent.getSocketById(m_data.session_id);
                    if (socketer != null) {
                        try {
                            //发送消息
                            socketer.write(sendRawData);
                        } catch (IOException e) {
                            logger.error("数据发送异常，连接可能已经关闭!");
                        }
                    }
                }else if(parent.getParent().getLoginUser(user.getId())!=null){
                    logger.warn("用户:"+login_user.getNickname()+"重复登录");
                    //重复登录
                    DataPackage<String> sendData=new DataPackage<String>(m_data.session_id,m_name, SocketPackgeType.response);
                    sendData.contents.add("OVER_LOGIN");
                    //转换数据
                    String sendRawData=gson.toJson(sendData,new TypeToken<DataPackage<String>>(){}.getType());
                    Socketer socketer=parent.getSocketById(m_data.session_id);
                    if(socketer!=null) {
                        try {
                            //发送消息
                            socketer.write(sendRawData);
                        } catch (IOException e) {
                            logger.error("数据发送异常，连接可能已经关闭!");
                        }
                    }
                } else{
                    parent.mapSocketToUser(m_data.session_id, user);
                    logger.info("用户:" + user.getNickname() + "完成登录");
                    //通知客户端并返回相关内容
                    DataPackage<User> sendData = new DataPackage<User>(m_data.session_id, m_name, SocketPackgeType.response);
                    sendData.contents.add(user);
                    String sendRawData = gson.toJson(sendData, new TypeToken<DataPackage<User>>() {
                    }.getType());
                    Socketer socketer = parent.getSocketById(m_data.session_id);
                    if (socketer != null) {
                        try {
                            //发送消息
                            socketer.write(sendRawData);
                        } catch (IOException e) {
                            logger.error("数据发送异常，连接可能已经关闭!");
                        }
                    }
                }
            }
        }

        logger.info("UserLoginHandler结束");
    }
}
