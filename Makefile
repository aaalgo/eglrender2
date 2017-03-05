CXX = g++
ARM_CXX = arm-linux-gnueabihf-g++-4.6
CXXFLAGS += -g -O3 
LDFLAGS += 
LDLIBS += -lboost_program_options -lopencv_imgproc -lopencv_highgui -lopencv_core -lGLESv2 -lEGL -lX11 -lXext -lm
X86_FLAGS = -std=c++11
ARM_FLAGS = -std=c++0x -Iroot/usr/include -Iroot/usr/include/arm-linux-gnueabihf -Lroot/usr/lib -Lroot/usr/lib/arm-linux-gnueabihf -Lroot/lib/arm-linux-gnueabihf -L/usr/arm-linux-gnueabihf/lib -Wl,--rpath-link=root/usr/lib:root/usr/lib/arm-linux-gnueabihf:root/lib:root/lib/arm-linux-gnueabihf

all:	demo

demo:	demo.cpp eglrender.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $(X86_FLAGS) $^ $(LDLIBS)

demo.arm:	demo.cpp eglrender.cpp
	$(ARM_CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $(ARM_FLAGS) $^ $(LDLIBS)


docker:
	docker build -t eglrender .
