#!/bin/bash

export LD_LIBRARY_PATH=/opt/amjCom/lib:/opt/amj/lib

../../../DataProcessor/src/DataProcessor \
    --receiver-frames /frames:2:10000 \
    --framesize 64 64 \
    --fringeperiod -10 \
    --fringeperiod -5 \
    --sender-tracker 127.0.0.1:27001 \
    --sender-phasorviewer 127.0.0.1:27002
