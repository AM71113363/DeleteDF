# Project: AM71113363


CC   = gcc.exe -s
WINDRES = windres.exe
RES  = main.res
OBJ  = main.o crc32.o $(RES)
LINKOBJ  = main.o crc32.o $(RES)
LIBS =  -L"C:/Dev-Cpp/lib" -mwindows  
INCS =  -I"C:/Dev-Cpp/include" 
BIN  = DeleteDF.exe
CFLAGS = $(INCS)  
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before DeleteDF.exe all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o "DeleteDF.exe" $(LIBS)

main.o: main.c
	$(CC) -c main.c -o main.o $(CFLAGS)

crc32.o: crc32.c
	$(CC) -c crc32.c -o crc32.o $(CFLAGS)

main.res: main.rc 
	$(WINDRES) -i main.rc --input-format=rc -o main.res -O coff 
