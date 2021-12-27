package net.yurzi.data;

import com.alibaba.fastjson.annotation.JSONField;
import com.google.gson.annotations.SerializedName;

import java.math.BigDecimal;

public class Item {
    @SerializedName("id") @JSONField(name ="id")
    private int id; //商品的id
    @SerializedName("name") @JSONField(name ="name")
    private String name;    //商品的名称
    @SerializedName("pic") @JSONField(name ="pic")
    private String pic; //商品的图片*
    @SerializedName("price") @JSONField(name ="price")
    private BigDecimal price;   //商品的价格
    @SerializedName("amount") @JSONField(name ="amount")
    private int amount; //商品的数量
    @SerializedName("owner") @JSONField(name ="owner")
    private String owner;   //商品的拥有者和添加者

    public Item(String name) {
        this.name = name;
        this.pic="";
        this.price=new BigDecimal(0);    //零元购（笑
        this.amount=0;
        this.owner="Yurzi";  //默认拥有者
    }

    public Item(String name, BigDecimal price) {
        this.name = name;
        this.pic="";
        this.price = price;
        this.amount=0;
        this.owner="Yurzi";  //默认拥有者
    }

    public Item(String name, String pic, BigDecimal price, int amount, String owner) {
        this.name = name;
        this.pic = pic;
        this.price = price;
        this.amount = amount;
        this.owner = owner;
    }

    public Item(int id, String name, String pic, BigDecimal price, int amount, String owner) {
        this.id = id;
        this.name = name;
        this.pic = pic;
        this.price = price;
        this.amount = amount;
        this.owner = owner;
    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getPic() {
        return pic;
    }

    public void setPic(String pic) {
        this.pic = pic;
    }

    public BigDecimal getPrice() {
        return price;
    }

    public void setPrice(BigDecimal price) {
        this.price = price;
    }

    public int getAmount() {
        return amount;
    }

    public void setAmount(int amount) {
        this.amount = amount;
    }

    public String getOwner() {
        return owner;
    }

    public void setOwner(String owner) {
        this.owner = owner;
    }
}
