set -e
cd test
west build -b native_sim
./build/test/zephyr/zephyr.exe
