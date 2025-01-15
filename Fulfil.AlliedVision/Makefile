PROC=$(shell nproc)

.phony : all
all:
	$(shell ../scripts/build_date.sh)
	cd build && cmake .. && make

.phony : node1
node1:
	@rsync -avP build/stark_test fulfil@10.40.20.11:/home/starkey/stark_test

.phony : local
local:
	@rsync -avP build/stark_test starkey@192.168.1.10:~/code/Fulfil.ComputerVision/Fulfil.AlliedVision/build/stark_test

.phony : repack_perception_test
repack_perception_test:
	cmake -DCMAKE_BUILD_TYPE=Release -B build -S .
	cmake --build build --target repack_perception_test -- -j $(PROC)