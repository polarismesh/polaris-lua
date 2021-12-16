CC=gcc

INC= -I./third_party/polaris_cpp_sdk/include \
	-I./third_party/lua/include
LIB=-L./third_party/polaris_cpp_sdk/slib -lpolaris_api -lprotobuf -lm -lstdc++ -L./third_party/lua/lib/ -llua

CFLAGS= -g -Wall -fPIC
TARGET=./polariswrapper.so

OBJ+=./polaris_sdk_lua_wrapper.o

$(TARGET):$(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LIB) -shared -pthread -lz -lrt

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)
