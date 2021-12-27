package net.yurzi.dao;

import net.yurzi.data.User;
import org.apache.ibatis.annotations.Param;

import java.math.BigDecimal;
import java.util.List;

public interface UserMapper {
    //按nickname查找
    public User getUserByNickname(String nickname);
    //按id查找
    public User getUserById(int id);
    //获取用户列表
    public List<User> getUserAllList();
    public List<User> getUserList(int limit);

    //添加一个用户
    public void addUser(User user);
    //更新一个用户
    public void updateUser(User user);
    public void updateUserMoney(@Param("nickname") String nickname, @Param("money") BigDecimal money);
    public void removeUser(User user);
}
