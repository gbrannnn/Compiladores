SRC=src
OBJ=obj
BIN=bin

CFLAGS +=-W  -Wall -Wextra -Iinclude
CC=gcc
TARGET=$(BIN)/calc
RM=rm -rf

SRCS=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))

all: prologo $(TARGET)

prologo:
	@mkdir -p $(OBJ) 
	@mkdir -p $(BIN)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS) 

$(OBJ)/%.o: $(SRC)/%.c 
	${CC} ${CFLAGS} -c $< -o $@

$(OBJ):
	mkdir $@

$(BIN):
	mkdir $@

clean:
	$(RM) -rf $(BIN) $(OBJ) 
