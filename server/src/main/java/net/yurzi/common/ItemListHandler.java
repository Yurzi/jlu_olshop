package net.yurzi.common;

import com.alibaba.fastjson.JSON;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.reflect.TypeToken;
import net.yurzi.controller.SocketController;
import net.yurzi.dao.ItemMapper;
import net.yurzi.data.DataPackage;
import net.yurzi.data.Item;
import net.yurzi.network.SocketPackgeType;
import net.yurzi.network.Socketer;
import org.apache.ibatis.session.SqlSession;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.IOException;
import java.util.List;

public class ItemListHandler extends SocketHandler{
    private static final Logger logger= LogManager.getLogger(ItemListHandler.class);

    public ItemListHandler(String rawData, SocketController _parent) {
        super(rawData, _parent);
        m_name="ItemList";
    }

    public ItemListHandler(DataPackage data, SocketController _parent) {
        super(data, _parent);
        m_name="ItemList";
    }

    @Override
    protected void resolveRawData() {
        logger.info("解析数据...");
        m_data= JSON.parseObject(m_rawData,DataPackage.class);
        logger.info("请求类型为:"+m_data.type);
    }

    @Override
    public void run() {
        this.resolveRawData();
        logger.info("数据解析完成。");
        if (m_data.type.equals(SocketPackgeType.request)){
            //如果是获取商品列表的请求则发送商品列表
            SqlSession sqlSession=MybatisUitls.getSqlSession();
            ItemMapper itemMapper=sqlSession.getMapper(ItemMapper.class);
            logger.info("获取商品列表...");
            List<Item> items=itemMapper.getItemList();     //从数据库中获取商品列表

            //封装
            logger.info("数据封包与转码...");
            DataPackage<Item> dataPackage=new DataPackage<Item>(m_data.session_id,m_name,SocketPackgeType.response);
            if (items!=null){
                if (items.size()>0){
                    dataPackage.contents.addAll(items);
                }
            }
            //转化为Json字符串
            Gson gson=new GsonBuilder().serializeNulls().create();
            String sendRawData=gson.toJson(dataPackage,new TypeToken<DataPackage<Item>>(){}.getType());
            //发送内容
            logger.info("发送数据...");
            Socketer socketer=parent.getSocketById(m_data.session_id);
            if(socketer!=null) {
                try {
                    //发送消息
                    socketer.write(sendRawData);
                } catch (IOException e) {
                    logger.error("数据发送异常，连接可能已经关闭!");
                }
            }
            sqlSession.close();
        }
    }
}
