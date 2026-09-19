CC=C:\mingw32\bin\g++ # define the compiler to use
TARGET=microworld # define the name of the executable

# RELEASE
# CFLAGS= -c -std=c++17 -O3
# DEBUG
CFLAGS= -c -std=c++17 -g3

LFLAGS= -Llib -LC:\SFML\lib -LC:\mingw32\lib -lm -lsfml-graphics -lsfml-window -lsfml-system
SRCDIR = src
OBJDIR = obj
INCLUDE = -Iinc -IC:\SFML\include -IC:\mingw32\include
SOURCES = $(foreach d,$(SRCDIR),$(wildcard $(d)/*.cpp))
OBJECTS = $(foreach d,$(SRCDIR),$(subst $(d),$(OBJDIR),$(wildcard $(d)/*.cpp)))

# define list of objects
OBJS=$(subst .cpp,.o,$(OBJECTS))
# the target is obtained linking all .o files
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDE) $^ -o $@

purge: clean
	rm -f $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

show:
	@echo 'SOURCES:   ' $(SOURCES)
	@echo 'OBJECTS:   ' $(OBJECTS)
	@echo 'OBJS:      ' $(OBJS)

################################################################################
################################################################################
