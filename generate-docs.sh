# For some reason I could not get the Doxygen config to exclude scanning
# the external directory if the Doxyfile was in the root directory. Hence
# this solution.
cd doxygen-config
doxygen Doxyfile
cd ..