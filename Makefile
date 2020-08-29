# compile Q1.cpp
noodles: Q1.cpp
	g++ -lpthread -std=c++11 Q1.cpp -o noodles

# compile Q1_priority.cpp
priority_noodles: Q1_priority.cpp
	g++ -lpthread -std=c++11 Q1_priority.cpp -o priority_noodles

# compile Q1_extra_fork.cpp
extra_fork_noodles: Q1_extra_fork.cpp
	g++ -lpthread -std=c++11 Q1_extra_fork.cpp -o extra_fork_noodles

# run noodles with 5 philosophers
run_noodles: noodles
	./noodles 5

# run priority_noodles with 5 philosophers
run_priority_noodles: priority_noodles
	./priority_noodles 5

# run extra_fork_noodles with 5 philosophers
run_extra_fork_noodles:
	./extra_fork_noodles 5

# clean the directory
clean:
	rm -f noodles priority_noodles extra_fork_noodles
