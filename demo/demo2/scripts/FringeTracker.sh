#!/bin/bash

export LD_LIBRARY_PATH=/opt/amjCom/lib:/opt/amj/lib

../../../FringeTracker/src/FringeTracker --active 1 \
	  --baseline baseline1 64 1 2.5 4 5 1 \
	  --baseline baseline2 64 1 2.5 4 5 1 \
	  --receiver-phasors 127.0.0.1:27001 \
	  --sender-pspec 127.0.0.1:27006 100 \
	  --sender-movements 127.0.0.1:27004 \
          --sender-tracker-stats 127.0.0.1:27008 10 1000 \
	  --sender-tracker-controller 127.0.0.1:27009 1000 \
	  --receiver-tracker-controller 127.0.0.1:27010
