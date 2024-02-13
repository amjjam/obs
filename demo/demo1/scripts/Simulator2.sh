#!/bin/bash

export LD_LIBRARY_PATH=/opt/amj/lib:/opt/amjCom/lib

#    --sender-phasors 127.0.0.1:27001 \
#    --sender-phasors2 127.0.0.1:27002 100\
../../Simulator/src/simulator \
    --sender-frames /frames:2:250000 \
    --receiver-delaylines 127.0.0.1:27003 \
    --baseline baseline1 1 2 200 1 2.5 1 1 1 \
    --baseline baseline2 2 3 250 1.5 3 1 1 2 \
    --baseline baseline3 3 4 250 1.5 3 1 1 2 \
    --externaldelaymodel 2 square 50 10 \
    --externaldelaymodel 3 cos 50 10
