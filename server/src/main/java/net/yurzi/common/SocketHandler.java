package net.yurzi.common;

import net.yurzi.controller.SocketController;
import net.yurzi.data.DataPackage;

abstract public class SocketHandler extends Handler {
    protected SocketController parent;

    public SocketHandler(String rawData, SocketController _parent) {
        super(rawData);
        parent = _parent;
    }

    ;

    public SocketHandler(DataPackage data, SocketController _parent) {
        super(data);
        parent = _parent;
    }
}
