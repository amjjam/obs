
TOPTARGETS := all clean install uninstall

QMAKE=qmake

SUBDIRS := DataProcessor/src DelayController/src \
  DelaylineViewer/DelaylineViewer FrameViewer/FrameViewer \
  FringeTracker/src FringeTrackerViewer/FringeTrackerViewer \
  PowerSpectrumViewer/PowerSpectrumViewer Simulator/src \
  TrackerController/TrackController

$(TOPTARGETS): $(SUBDIRS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)

DataProcessor/src:
	$(MAKE) -C $@ $(MAKECMDGOALS)

DelayController/src:
	$(MAKE) -C $@ $(MAKECMDGOALS)

DelaylineViewer/DelaylineViewer:
	(cd $@; $(QMAKE))
	$(MAKE) -C $@ $(MAKECMDGOALS)

FrameViewer/FrameViewer:
	(cd $@; $(QMAKE))
	$(MAKE) -C $@ $(MAKECMDGOALS)

FringeTracker/src:
	$(MAKE) -C $@ $(MAKECMDGOALS)

FringeTrackerViewer/FringeTrackerViewer:
	(cd $@; $(QMAKE))
	$(MAKE) -C $@ $(MAKECMDGOALS)

PowerSpectrumViewer/PowerSpectrumViewer:
	(cd $@; $(QMAKE))
	$(MAKE) -C $@ $(MAKECMDGOALS)

Simulator/src:
	$(MAKE) -C $@ $(MAKECMDGOALS)

TrackerController/TrackerController:
	(cd $@; $(QMAKE))
	$(MAKE) -C $@ $(MAKECMDGOALD)
