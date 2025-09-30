PROC=$(shell nproc)

LOCAL_DOCKER_IMAGE=fulfil_dc_api
IMAGE_TAG=latest
BASE_IMAGE=gcr.io/fulfil-web/$(LOCAL_DOCKER_IMAGE)/base:latest
DOCKER_IMAGE=gcr.io/fulfil-web/$(LOCAL_DOCKER_IMAGE)/runner:$(IMAGE_TAG)


.PHONY : all
all:
	$(shell ../scripts/build_date.sh)
	cmake -DCMAKE_BUILD_TYPE=Release -B build -S .
	cmake --build build --target main -- -j $(PROC)

.PHONY : test_calibration
test_calibration:
	cmake -DCMAKE_BUILD_TYPE=Debug -B build -S .
	cmake --build build --target test_auto_tray_calibration -- -j $(PROC)

.PHONY : client
client:
	cmake -DCMAKE_BUILD_TYPE=Release -B build -H .
	cmake --build build --target dispense_test_client -- -j $(PROC)

.PHONY : offline_bag
offline_bag:
	cmake -DCMAKE_BUILD_TYPE=Release -B build -S .
	cmake --build build --target offline_test -- -j $(PROC)

.PHONY : offline_tray
offline_tray:
	cmake -DCMAKE_BUILD_TYPE=Release -B build -S .
	cmake --build build --target tray_offline_simulation -- -j $(PROC)

.PHONY : offline_all
offline_all:
	cmake -DCMAKE_BUILD_TYPE=Release -B build -S .
	cmake --build build --target tray_offline_simulation -- -j $(PROC)
	cmake --build build --target offline_test -- -j $(PROC)

.PHONY : deploy
deploy:
	rsync -avP build/app/main root@10.10.103.83:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/build/app/main
	rsync -avP build/app/main root@10.10.103.88:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/build/app/main
	rsync -avP build/app/main root@10.10.103.62:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/build/app/main
	# rsync -avP build/app/main root@10.10.103.83:/usr/bin/dc-test-main
	# rsync -avP build/app/main root@10.10.103.88:/usr/bin/dc-test-main
	# rsync -avP build/app/main root@10.10.103.62:/usr/bin/dc-test-main

.PHONY : stark
stark:	
	cmake --build build --target stark_test -- -j $(PROC)
	rsync -avP build/app/stark_test root@192.168.1.242:/usr/bin/dc-main

.PHONY : clean
clean:
	rm -r build && mkdir -p build && cd build && cmake .. && cd ..

.PHONY: update
update:
	@cd ../Fulfil.CPPUtils/src/comm && protoc depthCams.proto --grpc_out=. --plugin=protoc-gen-grpc=`which grpc_cpp_plugin`
	@cd ../Fulfil.CPPUtils/src/comm && protoc depthCams.proto --cpp_out=.
	@cd ../Fulfil.CPPUtils/src/comm && cp depthCams.pb.h ../../include/Fulfil.CPPUtils/comm/depthCams.pb.h
	@cd ../Fulfil.CPPUtils/src/comm && cp depthCams.grpc.pb.h ../../include/Fulfil.CPPUtils/comm/depthCams.grpc.pb.h

.PHONY : docker-update
docker-update:
	docker run -v $(shell dirname $(shell pwd))/:/home/fulfil/code/Fulfil.ComputerVision/ -w /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense $(BASE_IMAGE) make update

.PHONY : local
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


.PHONY : build
build:
	docker build --load -t $(LOCAL_DOCKER_IMAGE) -f ../Dockerfile.runner ..

.PHONY : up
# build was taking ~15 seconds, so we removed it from dependencies
up: #build
	docker compose up -d

.PHONY : down
down:
	docker compose down

.PHONY: reset
reset: down up

.PHONY : shell
shell: up
	docker compose exec -it $(LOCAL_DOCKER_IMAGE) bash

.PHONY : run
run: up
	docker compose exec -it $(LOCAL_DOCKER_IMAGE) ./Fulfil.Dispense/build/app/main

.PHONY : tag
tag: build
	docker tag $(LOCAL_DOCKER_IMAGE) $(LOCAL_DOCKER_IMAGE):$(IMAGE_TAG)

.PHONY : push-staging
push-staging: tag
	docker push $(DOCKER_IMAGE):$(IMAGE_TAG)
	docker push $(DOCKER_IMAGE):latest

.PHONY: check-npm-env
check-npm-env: ## Ensures that NPM_GITHUB_TOKEN environment variable is defined so we can pull private NPM packages from GitHub.
ifndef NPM_GITHUB_TOKEN
	$(error NPM_GITHUB_TOKEN is undefined. Please create an auth token with access \
                to GitHub NPM packages and export it as an environment variable.)
endif

.PHONY: image
image: check-npm-env ## Builds the docker image
	@echo "Building $(IMAGE_NAME):$(IMAGE_TAG)..."
ifdef SEMANTIC_RELEASE
	@# NOTE: Check if the 'SEMANTIC_RELEASE' env variable is set so we don't
	@# break other jobs that use 'make image'
	@# https://andrewlock.net/caching-docker-layers-on-serverless-build-hosts-with-multi-stage-builds---target,-and---cache-from/
	docker pull $(IMAGE_NAME):latest || true
	docker build --cache-from $(IMAGE_NAME):latest --build-arg NPM_GITHUB_TOKEN=$(NPM_GITHUB_TOKEN) -f $(DOCKER_FILE) -t $(IMAGE_NAME):$(IMAGE_TAG) .
	docker tag $(IMAGE_NAME):$(IMAGE_TAG) $(IMAGE_NAME):latest
else
	@# NOTE: leaving this here so we don't break other jobs
	docker build --cache-from $(IMAGE_NAME)/master:latest --cache-from $(IMAGE_NAME)/$(CLEAN_BRANCH_NAME):latest --build-arg NPM_GITHUB_TOKEN=$(NPM_GITHUB_TOKEN) -f $(DOCKER_FILE) -t $(IMAGE_NAME):$(IMAGE_TAG) .
endif

.PHONY: push
push:
	@echo "Pushing image to $(IMAGE_NAME):$(IMAGE_TAG)"
	docker push $(IMAGE_NAME):$(IMAGE_TAG)
ifdef SEMANTIC_RELEASE
	docker push $(IMAGE_NAME):latest
endif

.PHONY: release
release: ## Run semantic-release to auto-increment version based on commits.
	npx semantic-release
