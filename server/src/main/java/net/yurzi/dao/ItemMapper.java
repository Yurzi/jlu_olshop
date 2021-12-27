package net.yurzi.dao;

import net.yurzi.data.Item;
import org.apache.ibatis.annotations.Param;

import java.math.BigDecimal;
import java.util.List;

public interface ItemMapper {
    //增加商品
    public void addItem(Item item);
    public void addItemFull(Item item);
    //获取商品
    public List<Item> getItemList();
    public Item getItemById(int id);
    public List<Item> getItemByName(String name);
    public List<Item> getItemByOwner(String owner);
    //更新商品
    public void updateItem(Item item);
    public void updateItemPrice(@Param("id") int id, @Param("price") BigDecimal price);
    public void updateItemAmount(@Param("id") int id,@Param("amount") int amount);
    //删除商品
    public void removeItem(Item item);
    public void removeItemById(int id);
}
