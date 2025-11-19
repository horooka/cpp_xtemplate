g++ -g src/main.cpp src/utils.cpp -o cpp_xtemplate -Iinclude $(pkg-config --cflags --libs gtkmm-3.0) -Wall -Wextra && ./cpp_xtemplate
