CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

t_mutex1 : ./test/t_mutex1.cpp
	$(CXX) -o t_mutex1  $^ $(CXXFLAGS) -lpthread

t_mutex2 : ./test/t_mutex2.cpp
	$(CXX) -o t_mutex2  $^ $(CXXFLAGS) -lpthread

t_producerAndConsumer : ./test/t_producerAndConsumer.cpp
	$(CXX) -o t_producerAndConsumer  $^ $(CXXFLAGS) -lpthread

clean:
	rm -f t_mutex1 t_mutex2