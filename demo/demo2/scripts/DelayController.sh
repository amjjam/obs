#!/bin/bash

export LD_LIBRARY_PATH=/opt/amjCom/lib:/opt/amj/lib

../../../DelayController/src/DelayController \
    --receiver-movements 127.0.0.1:27004 \
    --sender-display 127.0.0.1:27007 100 \
    --sender-delaylines sim 6 127.0.0.1:27003
