# make -f Build.mk

IROHA_LIB=-L../../../src/out/Default/obj.target/src/ -liroha -lverilog_writer -lnumeric -liroha
IROHA_INC=-I../../../src/

all	:
	c++ -O3 -Wall -shared -std=c++11 -fPIC $(IROHA_INC) `python3 -m pybind11 --includes` native.cpp -o native`python3-config --extension-suffix` $(IROHA_LIB)
