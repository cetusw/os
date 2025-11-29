# tests/CMakeLists.txt (НОВАЯ, НАДЕЖНАЯ ВЕРСИЯ)
include(FetchContent)

# Скачиваем Google Test, если его еще нет
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)


# Включаем CTest, чтобы можно было запускать тесты командой 'ctest'
enable_testing()

# Создаем исполняемый файл для тестов
add_executable(run_tests MemorySimulatorTest.cpp)

# Привязываем нашу библиотеку "simulator" и GTest к тестам
# gtest_main уже включает в себя gtest и main() функцию для запуска тестов
target_link_libraries(run_tests PRIVATE memory-simulator gtest_main)

include(GoogleTest)
# Добавляем тесты в CTest, чтобы команда 'ctest' их нашла
gtest_discover_tests(run_tests)