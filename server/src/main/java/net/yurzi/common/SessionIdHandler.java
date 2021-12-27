package net.yurzi.common;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import net.yurzi.controller.SocketController;
import net.yurzi.data.DataPackage;
import net.yurzi.network.SocketPackgeType;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.IOException;

public class SessionIdHandler extends SocketHandler {

    private final static Logger logger = LogManager.getLogger(SessionIdHandler.class);

    public SessionIdHandler(String rawData, SocketController _parent) {
        super(rawData, _parent);
        m_name = "SessionId";
    }

    public SessionIdHandler(DataPackage data, SocketController _parent) {
        super(data, _parent);
        m_name = "SessionId";

    }

    @Override
    protected void resolveRawData() {
        logger.info("解析数据...");
        m_data = new DataPackage<String>(m_rawData, m_name, SocketPackgeType.response);
        if (m_data.session_id!=null)m_data.contents.add(m_data.session_id);
    }

    @Override
    public void run() {
        resolveRawData();
        Gson gson = new GsonBuilder().serializeNulls().create();
        logger.info("将数据转为字符串");
        m_rawData = gson.toJson(m_data);
        logger.info("发送数据...");
        try {
            parent.getSocketById(m_data.session_id).write(m_rawData);
        } catch (IOException e) {
            logger.error("数据发送异常!");
            e.printStackTrace();
        }
    }
}
