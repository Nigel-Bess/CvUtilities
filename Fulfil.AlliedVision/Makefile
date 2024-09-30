
.phony : all
all:
	cd build && cmake .. && make

.phony : node1
node1:
	@rsync -avP build/stark_test fulfil@10.40.20.11:/home/starkey/stark_test

.phony : local
local:
	@rsync -avP build/stark_test starkey@192.168.1.10:~/code/Fulfil.ComputerVision/Fulfil.AlliedVision/build/stark_test