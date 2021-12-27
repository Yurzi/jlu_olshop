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
import java.util.Map;

public class ItemDelHandler extends SocketHandler{
    private static final Logger logger= LogManager.getLogger(ItemDelHandler.class);
    public ItemDelHandler(String rawData, SocketController _parent) {
        super(rawData, _parent);
        m_name="ItemDel";
    }

    public ItemDelHandler(DataPackage data, SocketController _parent) {
        super(data, _parent);
        m_name="ItemDel";
    }

    public void updateClientItem(){
        //获取商品列表
        SqlSession sqlSession=MybatisUitls.getSqlSession();
        ItemMapper itemMapper=sqlSession.getMapper(ItemMapper.class);
        logger.info("获取商品列表...");
        List<Item> items=itemMapper.getItemList();
        //封装
        logger.info("数据封包与转码...");
        DataPackage<Item> dataPackage=new DataPackage<Item>(m_data.session_id,"ItemList", SocketPackgeType.response);
        if (items!=null){
            if (items.size()>0){
                dataPackage.contents.addAll(items);
            }
        }
        //转化为Json字符串
        Gson gson=new GsonBuilder().serializeNulls().create();
        String sendRawData=gson.toJson(dataPackage,new TypeToken<DataPackage<Item>>(){}.getType());
        sqlSession.close();
        for (Map.Entry<String,Socketer> entry:parent.getSocketList().entrySet()){
            //发送内容
            logger.info("发送数据...");
            Socketer socketer=entry.getValue();
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

    @Override
    protected void resolveRawData() {
        logger.info("解析数据...");
        m_data= JSON.parseObject(m_rawData,DataPackage.class);
        logger.info("请求类型为:"+m_data.type);
    }

    @Override
    public void run() {
        resolveRawData();
        logger.info("数据解析完成");
        if (m_data.type.equals(SocketPackgeType.request)){
            //如果是request请求
            //1. 取出内部的Item的id;
            Item delItem=JSON.parseObject((String) m_data.contents.get(0),Item.class);
            logger.info("处理商品ID为："+delItem.getId());
            //2。进行数据库操作
            SqlSession sqlSession=MybatisUitls.getSqlSession();
            ItemMapper itemMapper=sqlSession.getMapper(ItemMapper.class);
            if (itemMapper.getItemById(delItem.getId())!=null) {
                //若找到则删除对应的内容
                itemMapper.removeItemById(delItem.getId());
                //提交
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
                //全网更新
                updateClientItem();
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
