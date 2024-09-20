PROC=$(shell nproc)

LOCAL_DOCKER_IMAGE=fulfil_dc_api
IMAGE_TAG=latest
BASE_IMAGE=gcr.io/fulfil-web/$(LOCAL_DOCKER_IMAGE)/base:latest
DOCKER_IMAGE=gcr.io/fulfil-web/$(LOCAL_DOCKER_IMAGE)/runner:$(IMAGE_TAG)
DOCKER_FILE=./Dockerfile.runner

.PHONY : build
build:
	docker build --load -t $(LOCAL_DOCKER_IMAGE) -f $(DOCKER_FILE) .

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

.PHONY: login
login: check-login-env
	@echo "$${GCLOUD_SERVICE_KEY}" | docker login -u _json_key --password-stdin https://gcr.io

.PHONY: check-login-env
check-login-env: ## Ensures that the GCLOUD_SERVICE_KEY environment variable is defined so we can log in to our Docker registry.
ifndef GCLOUD_SERVICE_KEY
	$(error GCLOUD_SERVICE_KEY is undefined. Generate a Google Cloud access token and \
	export it in your shell before logging in. [Refer to \
	https://cloud.google.com/container-registry/docs/advanced-authentication])
endif

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
