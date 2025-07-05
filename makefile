CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

INCLUDES = -I./include    # 添加多个路径，空格分隔
CFLAGS = -Wall $(INCLUDES)         # 将路径整合到编译选项

t_mutex1 : ./test/t_mutex1.cpp
	$(CXX) -o t_mutex1  $^ $(CXXFLAGS) -lpthread

t_mutex2 : ./test/t_mutex2.cpp
	$(CXX) -o t_mutex2  $^ $(CXXFLAGS) -lpthread

# test for condition variables
t_cond1 : ./test/t_cond1.cpp
	$(CXX) -o t_cond1  $^ $(CXXFLAGS) -lpthread

t_semaphore1 : ./test/t_semaphore1.cpp
	$(CXX) -o t_semaphore1  $^ $(CXXFLAGS) -lpthread

t_mysql1 : ./test/t_mysql1.cpp
	$(CXX) -o t_mysql1  $^ $(CXXFLAGS) -lmysqlclient

valint-nc : ./test/valint-nc.cpp
	$(CXX) -o valint-nc  $^ $(CXXFLAGS) 

clean:
	rm -f t_mutex1 t_mutex2 t_cond1 t_semaphore1 t_mysql1 valint-nc