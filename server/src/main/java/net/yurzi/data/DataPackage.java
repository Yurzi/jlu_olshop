package net.yurzi.data;

import com.alibaba.fastjson.annotation.JSONField;
import com.google.gson.annotations.SerializedName;

import java.util.ArrayList;

//数据包
public class DataPackage<T> {
    @SerializedName("session") @JSONField(name ="session")
    public String session_id;  //id
    @SerializedName("time") @JSONField(name ="time")
    public long timestamp;     //时间戳
    @SerializedName("handler") @JSONField(name ="handler")
    public String handlerName; //对于的处理者的名称
    @SerializedName("type") @JSONField(name ="type")
    public String type;    //数据包类型
    @SerializedName("status") @JSONField(name ="status")
    public int status;     //状态码
    @SerializedName("contents") @JSONField(name ="contents")
    public ArrayList<T> contents=new ArrayList<T>();// 内容

    public DataPackage() {
    }

    public DataPackage(String input_session_id, String input_handlerName, String input_type) {
        session_id = new String(input_session_id);//初始化SessionId
        //初始化时间
        timestamp = System.currentTimeMillis();   //获取时间戳
        handlerName = new String(input_handlerName);
        type = new String(input_type);
        status = 200; //默认200状态码为Ok
    }

    public DataPackage(String input_session_id, String input_handlerName, String input_type, int input_status) {
        session_id = new String(input_session_id);//初始化SessionId
        //初始化时间
        timestamp = System.currentTimeMillis();   //获取时间戳
        handlerName = new String(input_handlerName);
        type = new String(input_type);
        status = input_status; //默认200状态码为Ok
    }
}
