PROC=$(shell nproc)

.phony : all
all:
	$(shell ../scripts/build_date.sh)
	cd build && cmake .. && make

.phony : tcs_perception_test
tcs_perception_test:
	cmake -DCMAKE_BUILD_TYPE=Release -B build -S .
	cmake --build build --target tcs_perception_test -- -j $(PROC)