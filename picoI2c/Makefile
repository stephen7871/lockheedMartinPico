.PHONY: firmware tests clean

firmware:
	rm -rf build
	mkdir -p build
	cd build && cmake .. && make -j4

tests:
	rm -rf build-host
	mkdir -p build-host
	cd build-host && cmake .. -DTARGET_PICO=OFF -DBUILD_HOST_TESTS=ON && ctest --output-on-failure

clean:
	rm -rf build build-host