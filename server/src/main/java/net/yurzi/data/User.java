package net.yurzi.data;


import com.alibaba.fastjson.annotation.JSONField;
import com.google.gson.annotations.SerializedName;

import java.math.BigDecimal;

public class User {
    @SerializedName("id") @JSONField(name ="id")
    private int id;    //用户id
    @SerializedName("nickname") @JSONField(name ="nickname")
    private String nickname;  //用户名
    @SerializedName("password") @JSONField(name ="password")
    private String password;  //密码
    @SerializedName("privilege") @JSONField(name ="privilege")
    private int privilege; //权限
    @SerializedName("money") @JSONField(name ="money")
    private BigDecimal money; //金钱
    @SerializedName("icon") @JSONField(name ="icon")
    private String icon;    //头像

    public User(){
        nickname="";
        password="";
        privilege=Privilege.customer;
        money=new BigDecimal(0);
        icon="";
    }
    public User(String nickname, String password){
        this.nickname=nickname;
        this.password=password;
        privilege=Privilege.customer;
        money=new BigDecimal(0);
        icon="";
    }

    public User(String nickname, String password, int privilege, BigDecimal money, String icon) {
        this.id = id;
        this.nickname = nickname;
        this.password = password;
        this.privilege = privilege;
        this.money = money;
        this.icon = icon;
    }

    public User(int id, String nickname, String password, int privilege, BigDecimal money, String icon) {
        this.id = id;
        this.nickname = nickname;
        this.password = password;
        this.privilege = privilege;
        this.money = money;
        this.icon = icon;
    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public String getNickname() {
        return nickname;
    }

    public void setNickname(String nickname) {
        this.nickname = nickname;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(String password) {
        this.password = password;
    }

    public int getPrivilege() {
        return privilege;
    }

    public void setPrivilege(int privilege) {
        this.privilege = privilege;
    }

    public BigDecimal getMoney() {
        return money;
    }

    public void setMoney(BigDecimal money) {
        this.money = money;
    }

    public String getIcon() {
        return icon;
    }

    public void setIcon(String icon) {
        this.icon = icon;
    }

}
