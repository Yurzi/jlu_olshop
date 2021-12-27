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

public class UserDelHandler extends SocketHandler{
    private static final Logger logger= LogManager.getLogger(UserDelHandler.class);
    public UserDelHandler(String rawData, SocketController _parent) {
        super(rawData, _parent);
        m_name="UserDel";
    }

    public UserDelHandler(DataPackage data, SocketController _parent) {
        super(data, _parent);
        m_name="UserDel";
    }

    @Override
    protected void resolveRawData() {
        logger.info("解析数据...");
        m_data= JSON.parseObject(m_rawData,DataPackage.class);
        logger.info("请求类型为:"+m_data.type);
    }

    @Override
    public void run() {
        resolveRawData();
        logger.info("数据解析完成.");
        if (m_data.type.equals(SocketPackgeType.request)){
            //如果是request请求
            //1. 取出数据中的对象
            User delUser=JSON.parseObject((String) m_data.contents.get(0),User.class);
            logger.info("处理用户:"+delUser.getNickname());
            SqlSession sqlSession=MybatisUitls.getSqlSession();
            UserMapper userMapper=sqlSession.getMapper(UserMapper.class);
            User findUser=userMapper.getUserByNickname(delUser.getNickname());
            if (findUser!=null){
                //如果存在这样的用户
                //检查是否已经登录
                userMapper.removeUser(findUser);
                if(parent.getParent().getLoginUser(findUser.getId())!=null) {
                    //若用户已经登录则删除相关链接
                    parent.getParent().rmLoginUser(findUser.getId());
                    parent.unmapSocketAndUser(parent.getMappedSessionByUser(findUser.getId()));
                }
                sqlSession.commit();
                //向客户端发送ACK
                DataPackage<String> dataPackage=new DataPackage<String>(m_data.session_id,m_name,SocketPackgeType.response);
                dataPackage.contents.add("ACK");
                Gson gson=new GsonBuilder().serializeNulls().create();
                String sendRawData=gson.toJson(dataPackage,new TypeToken<DataPackage<String>>(){}.getType());
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
                //若未找到则发送NOT_FIND
                DataPackage<String> dataPackage=new DataPackage<String>(m_data.session_id,m_name,SocketPackgeType.response);
                dataPackage.contents.add("NOT_FIND");
                Gson gson=new GsonBuilder().serializeNulls().create();
                String sendRawData=gson.toJson(dataPackage,new TypeToken<DataPackage<String>>(){}.getType());
                Socketer socketer=parent.getSocketById(m_data.session_id);
                if(socketer!=null) {
                    try {
                        //发送消息
                        socketer.write(sendRawData);
                    } catch (IOException e) {
                        logger.error("数据发送异常，连接可能已经关闭!");
                    }
                }
            }
            sqlSession.close();
        }
    }
}
