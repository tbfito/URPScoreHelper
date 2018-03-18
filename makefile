CC = gcc
CFLAGS = -O3 -Wall -std=c++11

DIR_SRC := ./URPScoreHelper
DIR_OBJ := ./obj

TARGET := URPScoreHelper.exe

OBJS =	$(DIR_OBJ)/headers.o\
		$(DIR_OBJ)/gbkutf8.o\
		$(DIR_OBJ)/General.o\
		$(DIR_OBJ)/CCurlTask.o\
		$(DIR_OBJ)/StringHelper.o\
		$(DIR_OBJ)/Encrypt.o\
		$(DIR_OBJ)/Admin.o\
		$(DIR_OBJ)/OAuth2.o\
		$(DIR_OBJ)/URPRequests.o\
		$(DIR_OBJ)/URPScoreHelper.o\
		$(DIR_OBJ)/AES.o\
		$(DIR_OBJ)/INIReader.o\
		$(DIR_OBJ)/ini.o\
		$(DIR_OBJ)/main.o

LIBS =  -lstdc++ -lcurl -lfcgi++ -lfcgi -lm -lpthread -ldl -lmariadbclient

$(DIR_OBJ)/%.o : $(DIR_SRC)/%.cpp
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

clean:
	rm -f $(DIR_OBJ)/*.o $(TARGET)