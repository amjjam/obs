
TOPTARGETS := all clean install uninstall

QMAKE=qmake

include Makefile.obs

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

dist: dist_gather dist_degit dist_write dist_zip dist_clean

# Gather all the code pieces needed
# check out the appropriate version of each package.
# Right now that is only done with amjCom
dist_gather:
	- rm -rf $(prefix_dist)
	mkdir $(prefix_dist)
	git clone git@github.com:amjjam/obs.git $(prefix_dist)
	mkdir $(prefix_dist)/packages
	git clone git@github.com:amjjam/amjCom.git $(prefix_packages)/amjCom
	(cd $(prefix_packages)/amjCom; git checkout tags/v0.1.0 --quiet)
	git clone git@github.com:amjjam/amjFourier.git $(prefix_packages)/amjFourier
	git clone git@github.com:amjjam/amjTime.git $(prefix_packages)/amjTime
	git clone git@github.com:amjjam/amjChart.git $(prefix_packages)/amjChart
	git clone git@github.com:amjjam/QCustomPlot.git $(prefix_packages)/QCustomPlot
	git clone git@github.com:amjjam/amjWidgets.git $(prefix_packages)/amjWidgets

# Remove .git directories
dist_degit:
	(cd $(prefix_dist); find . -type d -name ".git" -exec echo rm -rf {} \;)

# Write a new Makefile for building the combined packages
dist_write:
	mv $(prefix_dist)/Makefile.install $(prefix_dist)/Makefile
	cat $(prefix_dist)/Makefile.obs >> $(prefix_dist)/Makefile

# Remove other things that should not be in the distribution
dist_rm:
	rm $(prefix_dist)/Makefile.obs

# Create a zip file
dist_zip:
	(cd $(prefix_dist_workdir); zip -9 -r $(dist_name).zip $(dist_name))

# Clean up so only the zip file is left
dist_clean:
	- rm -rf $(prefix_dist)
