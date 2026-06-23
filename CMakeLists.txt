cmake_minimum_required(VERSION 3.10)
project(MotorSocializare C)

set(CMAKE_C_STANDARD 99)

if(WIN32)
    # Libraria de retea necesara pe Windows pentru Mongoose
    link_libraries(ws2_32)
endif()

# 1. Spunem compilatorului unde gaseste fisierele header (precum mysql.h)
include_directories(mysql/include)

# 2. Spunem compilatorului unde gaseste fisierele de legatura (libmysql.lib)
link_directories(mysql/lib)

# 3. Compilam fisierele noastre sursa
add_executable(server main.c mongoose.c)

# 4. Legam programul final de libraria MySQL
target_link_libraries(server libmysql)