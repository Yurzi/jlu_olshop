package net.yurzi.controller;

import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.LogManager;

import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Main {
    public static final Logger logger = LogManager.getLogger(Main.class);

    public static void main(String[] args) throws Exception {
        logger.info("---------------------------------------------------------------------------");
        logger.info("                              现在是启动时时间!                              ");
        logger.info("---------------------------------------------------------------------------");
        logger.info("启动服务端.....");
        logger.info("启动控制器.....");
        Controller controller = new Controller();
        Thread controller_thread = new Thread(controller);
        controller_thread.setName("Controller");
        controller_thread.start();
        logger.info("完成,输入 \"exit\" 来退出");
        BufferedReader br=new BufferedReader(new InputStreamReader(System.in));
        String str=new String("");
        while (controller.getRunable()) {
            str = br.readLine();
            if(str==null)continue;
            if (str.equals("exit")) {
                controller.setRunable(false);
                controller.exit();
            }
        }
        logger.info("成功退出");
    }
}
