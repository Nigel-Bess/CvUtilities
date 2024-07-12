

.phony : all
all:
	cmake -DCMAKE_BUILD_TYPE=Release -B build
	cmake --build build --target main -- -j 6

.phony : calibrate
calibrate:
	cmake -DCMAKE_BUILD_TYPE=Release -B build
	cmake --build build --target auto_tray_calibration -- -j 6

.phony : client
client:
	cmake -DCMAKE_BUILD_TYPE=Release -B build
	cmake --build build --target dispense_test_client -- -j 6

.phony : deploy
deploy:
	rsync -avP build/app/main root@10.10.103.83:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/build/app/main
	rsync -avP build/app/main root@10.10.103.88:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/build/app/main
	rsync -avP build/app/main root@10.10.103.62:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/build/app/main
	# rsync -avP build/app/main root@10.10.103.83:/usr/bin/dc-test-main
	# rsync -avP build/app/main root@10.10.103.88:/usr/bin/dc-test-main
	# rsync -avP build/app/main root@10.10.103.62:/usr/bin/dc-test-main

.phony : stark
stark:	
	cmake --build build --target stark_test -- -j 6
	rsync -avP build/app/stark_test root@192.168.1.242:/usr/bin/dc-main

.phony : clean
clean:
	rm -r build && mkdir -p build && cd build && cmake .. && cd ..

.PHONY: update
update:
	@cd ../Fulfil.CPPUtils/src/comm && protoc depthCams.proto --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin`
	@cd ../Fulfil.CPPUtils/src/comm && protoc depthCams.proto --cpp_out=.
	@cd ../Fulfil.CPPUtils/src/comm && cp depthCams.pb.h ../../include/Fulfil.CPPUtils/comm/depthCams.pb.h
	@cd ../Fulfil.CPPUtils/src/comm && cp depthCams.grpc.pb.h ../../include/Fulfil.CPPUtils/comm/depthCams.grpc.pb.h

.phony : local
local: 
	rsync -avp build/app/main root@192.168.1.242:/usr/bin/dc-main
	# rsync -avP src/ root@192.168.1.242:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/src
	# rsync -avP include/ root@192.168.1.242:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/include
	# rsync -avP libs/Fulfil.CPPUtils/src root@192.168.1.242:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/libs/Fulfil.CPPUtils/src
	# rsync -avP libs/Fulfil.CPPUtils/include/ root@192.168.1.242:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/libs/Fulfil.CPPUtils/include
	# rsync -avP libs/Fulfil.MongoCpp/src/ root@192.168.1.242:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/libs/Fulfil.MongoCpp/src
	# rsync -avP libs/Fulfil.MongoCpp/include/ root@192.168.1.242:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/libs/Fulfil.MongoCpp/include
	# rsync -avP libs/Fulfil.DepthCam/src/ root@192.168.1.242:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/libs/Fulfil.DepthCam/src
	# rsync -avP libs/Fulfil.DepthCam/include/ root@192.168.1.242:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/libs/Fulfil.DepthCam/include
