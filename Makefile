
TOPTARGETS := packages all clean distclean install uninstall 

QMAKE=qmake6

SUBDIRS_OBS := DataProcessor/src DelayController/src \
  DelaylineViewer/DelaylineViewer FrameViewer/FrameViewer \
  SNRViewer/SNRViewer \
  FringeTracker/src FringeTrackerViewer/FringeTrackerViewer \
  PhasorViewer/PhasorViewer PowerSpectrumViewer/PowerSpectrumViewer \
  Simulator/src TrackerController/TrackerController Obs/Obs obs_script

$(TOPTARGETS): $(SUBDIRS_OBS)

.PHONY: $(TOPTARGETS) $(SUBDIRS_OBS)

DataProcessor/src:
	$(MAKE) -C $@ $(MAKECMDGOALS)

DelayController/src:
	$(MAKE) -C $@ $(MAKECMDGOALS)

DelaylineViewer/DelaylineViewer:
	(cd $@; $(QMAKE) )
	$(MAKE) -C $@ $(MAKECMDGOALS)

FrameViewer/FrameViewer:
	(cd $@; $(QMAKE) )
	$(MAKE) -C $@ $(MAKECMDGOALS)

SNRViewer/SNRViewer:
	(cd $@; $(QMAKE) )
	$(MAKE) -C $@ $(MAKECMDGOALS)

FringeTracker/src:
	$(MAKE) -C $@ $(MAKECMDGOALS)

FringeTrackerViewer/FringeTrackerViewer:
	(cd $@; $(QMAKE) )
	$(MAKE) -C $@ $(MAKECMDGOALS)

PhasorViewer/PhasorViewer:
	(cd $@; $(QMAKE) )
	$(MAKE) -C $@ $(MAKECMDGOALS)

PowerSpectrumViewer/PowerSpectrumViewer:
	(cd $@; $(QMAKE) )
	$(MAKE) -C $@ $(MAKECMDGOALS)

Simulator/src:
	$(MAKE) -C $@ $(MAKECMDGOALS)

TrackerController/TrackerController:
	(cd $@; $(QMAKE) )
	$(MAKE) -C $@ $(MAKECMDGOALS)

Obs/Obs:
	(cd $@; $(QMAKE) )
	$(MAKE) -C $@ $(MAKECMDGOALS)

# For target dist:
# copies all of obs to a different directory
# copies all of the libraries into a subdirectory of obs
# list of libraries to copy:
#   amjCom
#   amjFourier
#   amjTime
#   amjChart
#   qcustomplot
#   amjWidgets
# writes a new Makefile
#   the makefile contains a variable for the installation directory which
#   defaults to the same directory the makefile is in
#   the makefile will first build the libraries and install them
#   the makefile wil then build the obs code and install it
#   the makefile will write a Obs.sh script which contains full paths to
#   libraries and binary programs, which allows the Obs.sh to be located
#   anywhere

prefix_dist_workdir=/tmp
dist_name=obs
prefix_dist=$(prefix_dist_workdir)/$(dist_name)
prefix_packages=$(prefix_dist)/packages

dist: dist_gather dist_degit dist_write dist_rm dist_zip dist_clean

# Gather all the code pieces needed
# check out the appropriate version of each package.
# Right now that is only done with amjCom
dist_gather:
	- rm -rf $(prefix_dist)
	mkdir $(prefix_dist)
	cp -r ./* $(prefix_dist)
#	git clone git@github.com:amjjam/obs.git $(prefix_dist)
	mkdir $(prefix_dist)/packages
	git clone git@github.com:amjjam/amjCom.git $(prefix_packages)/amjCom
	(cd $(prefix_packages)/amjCom; git checkout tags/v0.8_20250708 --quiet)
	git clone git@github.com:amjjam/amjInterferometry.git $(prefix_packages)/amjInterferometry
	git clone git@github.com:amjjam/amjFourier.git $(prefix_packages)/amjFourier
	(cd $(prefix_packages)/amjFourier; git checkout tags/v0.15_20250706 --quiet)
	git clone git@github.com:amjjam/amjTime.git $(prefix_packages)/amjTime
	git clone git@github.com:amjjam/amjChart.git $(prefix_packages)/amjChart
	git clone git@github.com:amjjam/QCustomPlot.git $(prefix_packages)/QCustomPlot
	git clone git@github.com:amjjam/amjWidgets.git $(prefix_packages)/amjWidgets
	git clone git@github.com:amjjam/amjAtmosphere.git $(prefix_packages)/amjAtmosphere
	git clone git@github.com:amjjam/amjRandom.git $(prefix_packages)/amjRandom

# Remove .git directories
dist_degit:
	- find $(prefix_dist) -type d -name ".git" -exec rm -rf {} \;

# Write a new Makefile for building the combined packages
dist_write:
	mv $(prefix_dist)/Makefile.install $(prefix_dist)/Makefile
	cat $(prefix_dist)/Makefile.obs >> $(prefix_dist)/Makefile

# Remove other things that should not be in the distribution
dist_rm:
	- rm $(prefix_dist)/Makefile~
	rm $(prefix_dist)/Makefile.obs
	- rm $(prefix_dist)/Makefile.obs~
	- rm $(prefix_dist)/Makefile.install~
	find $(prefix_dist) -type f -name "*~" -delete
	find $(prefix_dist) -type f -name "*.o" -delete

# Create a zip file
dist_zip:
	- (cd $(prefix_dist_workdir); rm $(dist_name).zip)
	(cd $(prefix_dist_workdir); zip -9 -r $(dist_name).zip $(dist_name))

# Clean up so only the zip file is left
dist_clean:
	- rm -rf $(prefix_dist)
