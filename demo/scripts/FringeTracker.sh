#!/bin/bash

export LD_LIBRARY_PATH=/opt/amjCom/lib

../../FringeTracker/src/tracker --baseline baseline1 200 1 2.5 4 5 1 \
	  --baseline baseline2 250 1.5 3 4 5 1 \
	  --receiver-phasors 127.0.0.1:27001 \
	  --sender-pspec 127.0.0.1:27006 1000 \
	  --sender-movements 127.0.0.1:27005
