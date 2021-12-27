package net.yurzi.common;

import com.alibaba.fastjson.JSON;
import net.yurzi.controller.SocketController;
import net.yurzi.data.DataPackage;
import net.yurzi.data.User;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class UserLogoutHandler extends SocketHandler{
    private static final Logger logger= LogManager.getLogger(UserLogoutHandler.class);

    public UserLogoutHandler(String rawData, SocketController _parent) {
        super(rawData, _parent);
        m_name="UserLogout";
    }

    public UserLogoutHandler(DataPackage data, SocketController _parent) {
        super(data, _parent);
        m_name="UserLogout";
    }

    @Override
    protected void resolveRawData() {
        logger.info("解析数据....");
        m_data= JSON.parseObject(m_rawData,DataPackage.class);
        logger.info("请求类型为:"+m_data.type);
    }

    @Override
    public void run() {
        this.resolveRawData();
        logger.info("解析数据完成");
        User logout_user=JSON.parseObject((String) m_data.contents.get(0),User.class);
        logger.info("处理用户:"+logout_user.getNickname());
        parent.unmapSocketAndUser(m_data.session_id);
        logger.info("处理完毕");
    }
}
