# --- --- --- --- --- 
ENV := gdb-debug
ENV := no-debug
ENV := valgrind-debug
# --- --- --- --- --- 
VALGRIND_TOOL=helgrind
VALGRIND_TOOL=memcheck --leak-check=full --show-leak-kinds=all
VALGRIND_TOOL=memcheck --track-origins=yes --leak-check=full --show-leak-kinds=all --track-fds=yes -s
VALGRIND_TOOL=memcheck --track-origins=yes --leak-check=full --track-fds=yes -s
# --- --- --- --- --- 
STATIC_FLAGS := -static # enable static
STATIC_FLAGS := # disable static
DISABLE_WARNINGS := -Wno-unused-function
SERVER_LINKS := -lmicrohttpd -lgnutls
OAUTH_LINKS := -lcurl 
# --- --- --- --- --- 
libtest_path=./test
libexamples_path=./examples
libinclude_path=./lib/include
# --- --- --- --- --- 
GPP := g++
GCC := gcc
# --- --- --- --- --- 
ifeq ($(ENV),gdb-debug)
GPP += -Wall -g -O0 $(DISABLE_WARNINGS) $(STATIC_FLAGS) 
GCC += -Wall -g -O0 $(DISABLE_WARNINGS) $(STATIC_FLAGS) 
else
ifeq ($(ENV),valgrind-debug)
GPP += -Wall -g -O0 $(DISABLE_WARNINGS) $(STATIC_FLAGS) 
GCC += -Wall -g -O0 $(DISABLE_WARNINGS) $(STATIC_FLAGS) 
else
GPP += -Wall $(DISABLE_WARNINGS) $(STATIC_FLAGS) 
GCC += -Wall $(DISABLE_WARNINGS) $(STATIC_FLAGS) 
endif
endif
HEADERS := \
	-I $(libinclude_path)/ 
# --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- 
clean:
	rm -f ./build/*.o
# --- --- --- --- --- 
m_oauth_server:
	$(GCC) $(HEADERS) \
	$(libinclude_path)/../oauth_server.c $(SERVER_LINKS) -o ./build/oauth_server.o
# --- --- --- --- --- 
server:
	make m_oauth_server
	echo "building [oauth_server]..."
ifeq ($(ENV),gdb-debug)
	gdb ./build/oauth_server.o
else
ifeq ($(ENV),valgrind-debug)
	valgrind --tool=$(VALGRIND_TOOL) ./build/oauth_server.o
else
	./build/oauth_server.o
endif
endif
# --- --- --- --- --- 
m_oauth_test:
	$(GCC) $(HEADERS) \
	$(libexamples_path)/oauth2example.c $(OAUTH_LINKS) -o ./build/oauth2example.o
# --- --- --- --- --- 
oauth_test:
	make m_oauth_test
	echo "building [oauth2example]..."
ifeq ($(ENV),gdb-debug)
	gdb ./build/oauth2example.o
else
ifeq ($(ENV),valgrind-debug)
	valgrind --tool=$(VALGRIND_TOOL) ./build/oauth2example.o
else
	./build/oauth2example.o
endif
endif
# --- --- --- --- --- 
m_queue_test:
	$(GCC) $(HEADERS) \
	$(libtest_path)/queue_test.c $(OAUTH_LINKS) -o ./build/queue_test.o
# --- --- --- --- --- 
queue_test:
	make m_queue_test
	echo "building [queue_test]..."
ifeq ($(ENV),gdb-debug)
	gdb ./build/queue_test.o
else
ifeq ($(ENV),valgrind-debug)
	valgrind --tool=$(VALGRIND_TOOL) ./build/queue_test.o
else
	./build/queue_test.o
endif
endif
# --- --- --- --- --- 