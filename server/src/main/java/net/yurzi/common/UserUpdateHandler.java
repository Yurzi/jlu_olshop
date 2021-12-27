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

public class UserUpdateHandler extends SocketHandler{
    private static final Logger logger=LogManager.getLogger(UserUpdateHandler.class);

    public UserUpdateHandler(String rawData, SocketController _parent) {
        super(rawData, _parent);
        m_name="UserUpdate";
    }

    public UserUpdateHandler(DataPackage data, SocketController _parent) {
        super(data, _parent);
        m_name="UserUpdate";
    }

    public void updateClientUser(User user){
        //获取User对应的Socket
        String session=parent.getMappedSessionByUser(user.getId());
        Socketer socketer=parent.getSocketById(session);
        if (socketer!=null){
            //生成数据包 来源于UserLogin
            Gson gson=new GsonBuilder().serializeNulls().create();
            DataPackage<User> sendData = new DataPackage<User>(m_data.session_id, "UserLogin", SocketPackgeType.response);
            sendData.contents.add(user);
            String sendRawData = gson.toJson(sendData, new TypeToken<DataPackage<User>>() {
            }.getType());
            try {
                //发送消息
                socketer.write(sendRawData);
            } catch (IOException e) {
                logger.error("数据发送异常，连接可能已经关闭!");
            }
        }
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
                User upUser=JSON.parseObject((String) m_data.contents.get(0),User.class);
                logger.info("处理用户:"+upUser.getNickname());
                SqlSession sqlSession=MybatisUitls.getSqlSession();
                UserMapper userMapper=sqlSession.getMapper(UserMapper.class);
                User findUser=userMapper.getUserByNickname(upUser.getNickname());
                if (findUser!=null){
                    //若存在这样的用户 更新这个用户内容
                    findUser.setMoney(upUser.getMoney());
                    //更新数据库
                    userMapper.updateUserMoney(upUser.getNickname(),upUser.getMoney());
                    sqlSession.commit();
                    //检查是否登录
                    if(parent.getParent().getLoginUser(findUser.getId())!=null){
                        //若已经登录则需要进行更新
                        parent.getParent().getLoginUser(findUser.getId()).setMoney(findUser.getMoney());
                        //向远程客户端发信
                        this.updateClientUser(findUser);
                    }
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
