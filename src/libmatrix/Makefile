CXXFLAGS = -Wall -Werror -pedantic -O3
LIBMATRIX = libmatrix.a
LIBSRCS = mat.cc program.cc
LIBOBJS = $(LIBSRCS:.cc=.o)
TESTDIR = test
LIBMATRIX_TESTS = $(TESTDIR)/libmatrix_test
TESTSRCS = $(TESTDIR)/options.cc \
           $(TESTDIR)/const_vec_test.cc \
           $(TESTDIR)/inverse_test.cc \
           $(TESTDIR)/transpose_test.cc \
           $(TESTDIR)/libmatrix_test.cc
TESTOBJS = $(TESTSRCS:.cc=.o)

# Make sure to build both the library targets and the tests, and generate 
# a make failure if the tests don't pass.
default: $(LIBMATRIX) $(LIBMATRIX_TESTS) run_tests

# Main library targets here.
mat.o : mat.cc mat.h vec.h
program.o: program.cc program.h mat.h vec.h
libmatrix.a : mat.o mat.h stack.h vec.h program.o program.h
	$(AR) -r $@  $(LIBOBJS)

# Tests and execution targets here.
$(TESTDIR)/options.o: $(TESTDIR)/options.cc $(TESTDIR)/libmatrix_test.h
$(TESTDIR)/libmatrix_test.o: $(TESTDIR)/libmatrix_test.cc $(TESTDIR)/libmatrix_test.h $(TESTDIR)/inverse_test.h $(TESTDIR)/transpose_test.h
$(TESTDIR)/const_vec_test.o: $(TESTDIR)/const_vec_test.cc $(TESTDIR)/const_vec_test.h $(TESTDIR)/libmatrix_test.h vec.h
$(TESTDIR)/inverse_test.o: $(TESTDIR)/inverse_test.cc $(TESTDIR)/inverse_test.h $(TESTDIR)/libmatrix_test.h mat.h
$(TESTDIR)/transpose_test.o: $(TESTDIR)/transpose_test.cc $(TESTDIR)/transpose_test.h $(TESTDIR)/libmatrix_test.h mat.h
$(TESTDIR)/libmatrix_test: $(TESTOBJS) libmatrix.a
	$(CXX) -o $@ $^
run_tests: $(LIBMATRIX_TESTS)
	$(LIBMATRIX_TESTS)
clean :
	$(RM) $(LIBOBJS) $(TESTOBJS) $(LIBMATRIX) $(LIBMATRIX_TESTS)
