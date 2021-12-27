import net.yurzi.common.MybatisUitls;
import net.yurzi.dao.ItemMapper;
import net.yurzi.data.Item;
import org.apache.ibatis.session.SqlSession;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

import java.math.BigDecimal;
import java.util.List;

public class TestItemDao {
    @Test
    private void test(){
        SqlSession sqlSession= MybatisUitls.getSqlSession();
        ItemMapper itemMapper=sqlSession.getMapper(ItemMapper.class);

        Item item=new Item("Hello World!","", new BigDecimal(0),114514,"Yurzi");
        //添加商品
        itemMapper.addItem(item);
        //查询商品
        List<Item> res=itemMapper.getItemByOwner("Yurzi");
        Assertions.assertEquals(res.get(0).getName(),"Hello World!");

        itemMapper.removeItemById(res.get(0).getId());
    }
}
