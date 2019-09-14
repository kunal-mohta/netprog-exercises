FILE='./hello.txt'
CHILDREN=5

compile:
	@echo "Compiling program..."
	@gcc load_balancer.c
	@echo "Compiled program!\n"
	@echo "Run 'make run' to execute with default arguments"
	@echo "By default - file name = ${FILE} & Number of children = ${CHILDREN}\n"
	@echo "To run with custom arguments use -"
	@echo "'make run FILE=/path/to/custom/file CHILDREN=child_count'"

run: a.out
	@echo "Running program...\n\n"
	@./a.out ${FILE} ${CHILDREN}
