HEADERS = formula.h model.h dpll_solve.h
OBJECTS = main.o formula.o model.o dpll_solve.o

default: satcdcl

&.o: %.c $(HEADERS)
		gcc -c $< -o $@

satcdcl: $(OBJECTS)
	gcc $(OBJECTS) -o satcdcl

clean:
	-rm -f $(OBJECTS)
	-rm -f satcdcl

