set -e

# Build tests
cd test
west build -b native_sim
./build/test/zephyr/zephyr.exe
cd ..

# Build examples
cd examples/EventThreadExample
west build -b native_sim
./build/EventThreadExample/zephyr/zephyr.exe
cd ../..