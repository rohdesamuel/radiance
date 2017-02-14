# Directories
SRC_DIR = src
LIB_DIR = lib
OBJ_DIR = obj

# Versioning
MAJOR_VERSION = 1
MINOR_VERSION = 0
BUILD_VERSION = 0
VERSION = $(MAJOR_VERSION).$(MINOR_VERSION).$(BUILD_VERSION)

all:
	# Author: Samuel Rohde (rohde.samuel@gmail.com)
	#
	# Now building libradiance...
	# Current version is $(VERSION)
	g++ -c -fPIC -fno-exceptions $(SRC_DIR)/*.cpp -Wall -Wextra -Werror -std=c++14 -O3 -lSDL2 -lGLEW -lGL -lGLU -fopenmp
	g++ -shared -fPIC -fno-exceptions -Wl,-soname,libradiance.so.$(MAJOR_VERSION) -o $(LIB_DIR)/libradiance.so.$(VERSION) *.o -lc
	@ln -f -r -s $(LIB_DIR)/libradiance.so.$(VERSION) $(LIB_DIR)/libradiance.so.$(MAJOR_VERSION)
	@ln -f -r -s $(LIB_DIR)/libradiance.so.$(VERSION) $(LIB_DIR)/libradiance.so
	@mv *.o obj/

clean:
	@rm $(LIB_DIR)/*
	@rm $(OBJ_DIR)/*
