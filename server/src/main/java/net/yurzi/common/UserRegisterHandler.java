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

public class UserRegisterHandler extends SocketHandler {
    private static final Logger logger = LogManager.getLogger(UserRegisterHandler.class);

    public UserRegisterHandler(String rawData, SocketController _parent) {
        super(rawData, _parent);
        m_name = "UserRegister";
    }

    public UserRegisterHandler(DataPackage data, SocketController _parent) {
        super(data, _parent);
        m_name = "UserRegister";
    }

    @Override
    protected void resolveRawData() {
        logger.info("解析数据....");
        //logger.info("解析数据为:"+m_rawData);
        //创建json解析器
        m_data = JSON.parseObject(m_rawData, DataPackage.class);
        logger.info("请求类型为:" + m_data.type);
    }

    @Override
    public void run() {
        resolveRawData();   //解析数据
        logger.info("数据解析完成。");
        if (m_data.type.equals(SocketPackgeType.request)) {
            //如果是request，则代表请求注册
            //查找用户信息
            User register_user = JSON.parseObject((String) m_data.contents.get(0), User.class); //获取欲注册用户
            //查询数据库
            SqlSession sqlSession = MybatisUitls.getSqlSession();
            //logger.info("打开数据库连接");
            UserMapper userMapper = sqlSession.getMapper(UserMapper.class);
            logger.info("查找用户");
            User user = userMapper.getUserByNickname(register_user.getNickname()); //查找
            if (user == null) {
                //若不存在则运行注册
                User newUser=new User(register_user.getNickname(),register_user.getPassword());
                userMapper.addUser(newUser);    //往数据库写入
                sqlSession.commit();    //提交数据
                sqlSession.close();
                logger.info("用户:"+register_user.getNickname()+"注册成功");
                DataPackage<String> sendData=new DataPackage<String>(m_data.session_id,m_name, SocketPackgeType.response);
                sendData.contents.add("ACK");
                //转换数据
                Gson gson=new GsonBuilder().serializeNulls().create();
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
                DataPackage<String> sendData=new DataPackage<String>(m_data.session_id,m_name, SocketPackgeType.response);
                sendData.contents.add("REGISTERED");
                //转换数据
                Gson gson=new GsonBuilder().serializeNulls().create();
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
            }
        }
    }
}
