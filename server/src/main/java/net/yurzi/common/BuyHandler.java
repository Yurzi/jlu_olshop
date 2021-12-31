package net.yurzi.common;

import com.alibaba.fastjson.JSON;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.reflect.TypeToken;
import com.google.protobuf.MapEntry;
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
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.concurrent.locks.ReentrantLock;

public class BuyHandler extends SocketHandler{
    private static final Logger logger= LogManager.getLogger(BuyHandler.class);
    ReentrantLock buylock=new ReentrantLock();
    public BuyHandler(String rawData, SocketController _parent) {
        super(rawData, _parent);
        m_name="Buy";
    }

    public BuyHandler(DataPackage data, SocketController _parent) {
        super(data, _parent);
        m_name="Buy";
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
        logger.info("解析数据");
        m_data= JSON.parseObject(m_rawData,DataPackage.class);
        logger.info("请求的类型为:"+m_data.type);
    }

    @Override
    public void run() {
        resolveRawData();
        logger.info("数据解析完成");
        buylock.lock();
        if (m_data.type.equals(SocketPackgeType.request)){
            //查找用户
            logger.info("处理用户:"+parent.getMappedUser(m_data.session_id).getNickname()+"的事件");
            User user=parent.getMappedUser(m_data.session_id);
            SqlSession sqlSession=MybatisUitls.getSqlSession();
            //连接到数据库
            ItemMapper itemMapper=sqlSession.getMapper(ItemMapper.class);
            UserMapper userMapper=sqlSession.getMapper(UserMapper.class);
            if (user!=null){
                //进行购买操作
                //获取当前用户的金额
                BigDecimal originalMoney=new BigDecimal(0);
                //准备好未处理的列表
                List<Item> leftItems=new ArrayList<Item>();
                //准备好现金流处理队列
                Hashtable<String,BigDecimal> flowList=new Hashtable<String,BigDecimal>();
                for (int i=0;i<m_data.contents.size();++i){
                    //获取处理的item
                    Item procItem=JSON.parseObject((String) m_data.contents.get(i),Item.class);
                    //获取数据库中的item;
                    Item dataItem=itemMapper.getItemById(procItem.getId());
                    if (dataItem==null)continue;
                    logger.info("处理商品:"+dataItem.getName());
                    //比较数量
                    if (dataItem.getAmount()>=procItem.getAmount()){
                        //减去物品
                        dataItem.setAmount(dataItem.getAmount()-procItem.getAmount());
                        //累积要减去金额
                        originalMoney=originalMoney.add(dataItem.getPrice().multiply(new BigDecimal(procItem.getAmount())));
                        logger.info("累计的要减去的金额为："+originalMoney);
                        //找到该商品的owner
                        if(flowList.get(dataItem.getOwner())==null){
                            //加入该键值对
                            flowList.put(dataItem.getOwner(),dataItem.getPrice().multiply(new BigDecimal(procItem.getAmount())));
                        }else {
                            //否则则加上内容
                            BigDecimal res=flowList.get(dataItem.getOwner()).add(dataItem.getPrice().multiply(new BigDecimal(procItem.getAmount())));
                            flowList.remove(dataItem.getOwner());
                            flowList.put(dataItem.getOwner(),res);
                        }
                    }else {
                        //数目不够则不对该商品进行操作,并加入到不处理列表
                        logger.warn("商品:"+procItem.getName()+"购买失败");
                        leftItems.add(procItem);
                    }
                    //商品列表更新
                    logger.info("更新商品"+dataItem.getName()+"的数量为:"+dataItem.getAmount());
                    itemMapper.updateItemAmount(dataItem.getId(),dataItem.getAmount());
                    sqlSession.commit();
                }
                //用户列表更新
                BigDecimal tmp=originalMoney.negate();
                //logger.info("要减去的金额为"+tmp);
                logger.info("减去金额前的值");
                BigDecimal tmp2=user.getMoney().add(tmp);
                logger.info("减去金额后的值"+tmp2);
                user.setMoney(tmp2);
                logger.info("更新用户:"+user.getNickname()+"的余额为:"+user.getMoney());
                //向客户端发送数据包
                this.updateClientUser(user);
                userMapper.updateUser(user);
                //遍历现金流
                for(Map.Entry<String,BigDecimal> entry:flowList.entrySet()){
                    User targetUser=userMapper.getUserByNickname(entry.getKey());
                    if (targetUser!=null){
                        targetUser.setMoney(targetUser.getMoney().add(entry.getValue()));
                        logger.info("用户:"+targetUser.getNickname()+"余额:"+targetUser.getMoney());
                        userMapper.updateUserMoney(targetUser.getNickname(),targetUser.getMoney());
                        //如果更新的用户已经登录
                        if (parent.getParent().getLoginUser(targetUser.getId())!=null){
                            //更新登录用户信息
                            parent.getParent().getLoginUser(targetUser.getId()).setMoney(targetUser.getMoney());
                            //则远程更新这些客户端
                            this.updateClientUser(targetUser);
                        }
                    }
                }
                //提交更新
                sqlSession.commit();
                sqlSession.close();
                //检查是否有商品未处理 并通知客户端
                String sendRawData;
                Gson gson=new GsonBuilder().serializeNulls().create();
                if (!leftItems.isEmpty()){
                    //有商品为处理
                    //生成数据包并设置状态位
                    DataPackage<Item> dataPackage=new DataPackage<Item>(m_data.session_id,m_name,SocketPackgeType.response,206);
                    dataPackage.contents.addAll(leftItems);
                    //解析为Json
                    sendRawData=gson.toJson(dataPackage,new TypeToken<DataPackage<Item>>(){}.getType());
                }else {
                    //若商品都处理完则发送正常数据包
                    DataPackage<String> dataPackage=new DataPackage<String>(m_data.session_id,m_name,SocketPackgeType.response);
                    sendRawData=gson.toJson(dataPackage,new TypeToken<DataPackage<String>>(){}.getType());
                }
                //发送数据
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
        buylock.unlock();   //解锁
    }
}
