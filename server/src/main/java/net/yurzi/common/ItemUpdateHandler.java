package net.yurzi.common;

import com.alibaba.fastjson.JSON;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.reflect.TypeToken;
import net.yurzi.controller.SocketController;
import net.yurzi.dao.ItemMapper;
import net.yurzi.dao.UserMapper;
import net.yurzi.data.DataPackage;
import net.yurzi.data.Item;
import net.yurzi.data.User;
import net.yurzi.network.SocketPackgeType;
import net.yurzi.network.Socketer;
import org.apache.ibatis.session.SqlSession;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.IOException;
import java.math.BigDecimal;

public class ItemUpdateHandler extends SocketHandler{

    private static final Logger logger= LogManager.getLogger(ItemUpdateHandler.class);

    public ItemUpdateHandler(String rawData, SocketController _parent) {
        super(rawData, _parent);
        m_name="ItemUpdate";
    }

    public ItemUpdateHandler(DataPackage data, SocketController _parent) {
        super(data, _parent);
        m_name="ItemUpdate";
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
        SqlSession sqlSession=MybatisUitls.getSqlSession();
        ItemMapper itemMapper=sqlSession.getMapper(ItemMapper.class);
        UserMapper userMapper=sqlSession.getMapper(UserMapper.class);
        if (m_data.type.equals(SocketPackgeType.request)){
            //如果是请求则进行处理
            //1. 将内容物取出
            Item newItem=JSON.parseObject((String) m_data.contents.get(0),Item.class);
            //2. 判断id的值是否为-1；
            int rt=0;
            if (newItem.getId()==-1){
                rt=1;
            }
            //3. 判断owner是否存在
            User ownerUser=userMapper.getUserByNickname(newItem.getOwner());
            if (ownerUser==null){
                rt=2;
            }
            //进行负数检查
            if(newItem.getAmount()<0)newItem.setAmount(0);
            if (newItem.getPrice().compareTo(new BigDecimal(0)) < 0)newItem.setPrice(new BigDecimal(0));
            // 若rt=0 判断是否存在id冲突
            if (rt==0){
                if(itemMapper.getItemById(newItem.getId())!=null)rt=3;
            }
            //开始插入
            if (rt==0){
                //带有id的插入
                itemMapper.addItemFull(newItem);
            }else if (rt==1){
                //不带有id的插入
                itemMapper.addItem(newItem);
            }
            //提交
            sqlSession.commit();

            //结果输出
            Gson gson=new GsonBuilder().serializeNulls().create();
            DataPackage<String> dataPackage=new DataPackage<String>(m_data.session_id,m_name,SocketPackgeType.response);
            if (rt<=1){
                dataPackage.contents.add("ACK");
            }else if (rt==2){
                dataPackage.contents.add("OWNER_UNDEFINE");
            } else if (rt == 3) {
                dataPackage.contents.add("ID_CONFLICT");
            }
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
