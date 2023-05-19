#!/bin/bash

export LD_LIBRARY_PATH=/opt/amjCom/lib

../../Simulator/src/simulator \
    --sender-phasors 127.0.0.1:27001 \
    --sender-phasors 127.0.0.1:27002 \
    --baseline baseline1 2 1 200 1 2.5 4 5 1 \
    --baseline baseline2 3 2 250 1.5 3 4 5 2

	   
