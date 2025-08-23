CC=afl-gcc
CFLAGS=-g -O0 -Wall
WRAPPER=runner
TARGET=buggy
TARGETF=buggy_fortify

.PHONY: all build fuzz clean

all: fuzz

build: $(TARGET)

build_good: $(TARGETF)

$(TARGET): buggy.c
	$(CC) $(CFLAGS) -o $(TARGET) buggy.c

$(TARGETF): buggy.c
	$(CC) $(CFLAGS) -fsanitize=address -o $(TARGETF) buggy.c

# Build C wrapper for AFL
wrapper: runner.c $(TARGET)
	$(CC) $(CFLAGS) -o $(WRAPPER) runner.c

wrapper_f: runner.c $(TARGET)
	$(CC) $(CFLAGS) -DFORTIFY -o $(WRAPPER) runner.c


fuzz: build
	# Create input corpus if not exists
	rm -rf ./findings
	rm -rf inputs
	mkdir -p inputs
	echo "test" > inputs/in.txt
	# Run AFL++ fuzzing
	AFL_SKIP_CPUFREQ=1 AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 afl-fuzz -i inputs -o findings -- ./$(TARGET) @@

fuzz2: build
	# Create input corpus if not exists
	rm -rf ./findings
	rm -rf inputs
	mkdir -p inputs
	echo "test" > inputs/in.txt
	# Run AFL++ fuzzing
	AFL_SKIP_CPUFREQ=1 AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 afl-fuzz -t 2000 -i inputs -o findings -- ./$(TARGET) @@

fuzz3: build wrapper
	rm -rf ./findings
	# Create input corpus if not exists
	rm -rf inputs
	mkdir -p inputs
	echo "test" > inputs/in.txt
	echo "12345678" > inputs/in1.txt
	# Run AFL++ fuzzing
	AFL_SKIP_CPUFREQ=1 AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 afl-fuzz -t 2000 -i inputs -o findings -- ./runner.sh @@

fuzz4: build wrapper
	rm -rf ./findings
	# Create input corpus if not exists
	rm -rf inputs
	mkdir -p inputs
	echo "test" > inputs/in.txt
	echo "12345678" > inputs/in1.txt
	# Run AFL++ fuzzing
	AFL_SKIP_CPUFREQ=1 AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 afl-fuzz -t 2000 -i inputs -o findings -- ./runner @@

fuzz5: build_good wrapper_f
	rm -rf ./findings
	# Create input corpus if not exists
	rm -rf inputs
	mkdir -p inputs
	echo "test" > inputs/in.txt
	echo "12345678" > inputs/in1.txt
	# Run AFL++ fuzzing
	AFL_SKIP_CPUFREQ=1 AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 afl-fuzz -t 2000 -i inputs -o findings -- ./runner @@

clean:
	rm -f $(TARGET) $(TARGETF) $(WRAPPER)
	rm -rf inputs findings
	
