package net.yurzi.dao;

import net.yurzi.data.User;

public interface TableInit {
    public void initUserTable();    //初始化用户登录表
    public void initRootUser(User rootUser);    //初始化Root用户
    public User getRootUser(int privilege);     //获取Root用户
    public void initItemListTable();    //初始化商品表
}
