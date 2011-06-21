TARGETS = libmatrix.a matrix_inverse_test
SRCS = mat.cc program.cc matrix_inverse_test.cc
OBJS = $(SRCS:.cc=.o)
CXXFLAGS = -Wall -Werror -pedantic -O3

default: $(TARGETS)

mat.o : mat.cc mat.h
program.o: program.cc program.h
matrix_inverse_test.o: matrix_inverse_test.cc mat.h
matrix_inverse_test: matrix_inverse_test.o libmatrix.a
	$(CXX) -o $@ $?
libmatrix.a : mat.o mat.h stack.h vec.h program.o program.h
	$(AR) -r $@  $(OBJS)

clean :
	$(RM) $(OBJS) $(TARGETS)
