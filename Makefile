BX_INCLUDE_PATH=../bx/include/
BGFX_INCLUDE_PATH=../bgfx/include/
BGFX_BUILD_PATH=../bgfx/.build/linux64_gcc/bin/

noisebgfx: noisebgfx.cpp
	$(CXX) noisebgfx.cpp  -I$(BX_INCLUDE_PATH) -I$(BGFX_INCLUDE_PATH)  -L$(BGFX_BUILD_PATH) -lbgfx-shared-libRelease -lGL -lSDL2 -lbxRelease -std=c++11 -o noisebgfx
	echo Remember to run export LD_LIBRARY_PATH=$(BGFX_BUILD_PATH)

clean:
	rm -f noisebgfx
