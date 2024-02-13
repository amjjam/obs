#!/bin/bash

export LD_LIBRARY_PATH=/opt/amjCom/lib:/opt/amj/lib

../../DataProcessor/src/DataProcessor --baseline baseline1 200 1 2.5 2 7 -10 \
		--baseline baseline2 250 1.5 3 3 6 20 \
		--receiver-frames /frames:2:250000 \
		--sender-tracker 127.0.0.1:27001 \
		--sender-phasorviewer 127.0.0.1:27002
