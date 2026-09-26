CXX = g++

# --- WINDOWS (MSYS2 UCRT64) ---
SYSTEMC_HOME = /ucrt64
CXXFLAGS     = -std=c++20 -I. -I$(SYSTEMC_HOME)/include
LDFLAGS      = -L$(SYSTEMC_HOME)/lib -lsystemc -Wl,--allow-multiple-definition
TARGET       = my_vp.exe

# --- LINUX / WSL (Ubuntu) ---
# CXXFLAGS     = -std=c++17 -I. -I$(SYSTEMC_HOME)/include
# LDFLAGS      = -L$(SYSTEMC_HOME)/lib-linux64 -lsystemc
# TARGET       = my_vp


all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) main.cpp $(LDFLAGS) -o $(TARGET)

clean:
	rm -f my_vp my_vp.exe
