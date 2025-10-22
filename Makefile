CXX       = c++
CXXFLAGS  = -Wall -Wextra -std=c++98 -I$(INCLUDE)

NAME      = webserv

# Colors
GREEN   = \033[0;32m
RED     = \033[0;31m
NC      = \033[0m

# Include folder
INCLUDE   = include

# === src files =======================================
SRC       = src/ConfigFile.cpp \
			src/Location.cpp \
			src/paths.cpp \
			src/logging.cpp \
			src/ReadConfig.cpp \
			src/ServerUnit.cpp \
			src/main.cpp \
			src/statusCode.cpp \
			src/utils.cpp \
			src/Request.cpp \
			src/HttpResponse.cpp \
			src/ServerManager.cpp \
			src/Cgi.cpp \



# Build folder
BUILD_DIR = build

# Object files
OBJ       = $(SRC:src/%.cpp=$(BUILD_DIR)/%.o)

# === test files =======================================

TEST_SRC  = tests/test_configparser.cpp \
            tests/test_httpparser.cpp

TEST_BIN  = tests/test_configparser \
            tests/test_httpparser

# === Rules =======================================

# Default target
all: $(NAME)

# Link server binary
$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

# Compile object files into the build folder
$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@) # Create the directory structure if it doesn't exist
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Build test binaries
test: $(TEST_BIN)
	@for bin in $(TEST_BIN); do echo "Running $$bin"; ./$$bin; done

tests/test_configparser: tests/test_configparser.cpp $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $<

tests/test_httpparser: tests/test_httpparser.cpp $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $<

# Clean object files and test binaries
clean:
	rm -rf $(BUILD_DIR) $(TEST_BIN)

# Clean everything including main binary
fclean: clean
	rm -f $(NAME)

# Rebuild everything
re: fclean all

.PHONY: all clean fclean re test
