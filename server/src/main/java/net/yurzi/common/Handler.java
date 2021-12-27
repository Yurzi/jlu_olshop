package net.yurzi.common;

import net.yurzi.data.DataPackage;

abstract public class Handler implements Runnable {
    protected String m_name;    //handler的名字
    protected DataPackage m_data; //存放的数据内容
    protected String m_rawData; //字符串数据


    public Handler(String raw_data) {
        this.m_rawData = raw_data;//将数据送入data域
    }

    public Handler(DataPackage<?> data) {
        this.m_data = data;
    }

    public String getHandlerName() {
        return m_name;
    }

    public DataPackage<?> getData() {
        return m_data;
    }
    public void setRawData(String rawData){
        m_rawData=rawData;
    }

    protected abstract void resolveRawData();
}


