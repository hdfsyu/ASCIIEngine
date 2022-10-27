all:
  g++ -o ASCIIEngine.exe ASCIIEngine/ASCIIEngine.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17
