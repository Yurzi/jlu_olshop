package net.yurzi.common;

import net.yurzi.controller.SocketController;
import net.yurzi.data.DataPackage;

public class HeartHandler extends SocketHandler {

    public HeartHandler(String rawData, SocketController _parent) {
        super(rawData, _parent);
        resolveRawData();
    }

    public HeartHandler(DataPackage data, SocketController _parent) {
        super(data, _parent);
    }

    @Override
    protected void resolveRawData() {

    }

    @Override
    public void run() {

    }
}
