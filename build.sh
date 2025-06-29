set -e

source .venv/bin/activate

# Build and run tests
cd test
west build -b native_sim
./build/test/zephyr/zephyr.exe
cd ..

# Build examples, but don't run them
cd examples/EventThreadExample
west build -b native_sim
# ./build/EventThreadExample/zephyr/zephyr.exe
cd ../..

cd examples/Gpio
west build -b native_sim
# ./build/Gpio/zephyr/zephyr.exe
cd ../..