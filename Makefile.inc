LOCAL_IMAGE ?= openshift-spark-inc
SPARK_IMAGE=radanalyticsio/openshift-spark-inc
DOCKERFILE_CONTEXT=openshift-spark-build-inc

OPENSHIFT_SPARK_TEST_IMAGE ?= spark-testimage-inc
export OPENSHIFT_SPARK_TEST_IMAGE

.PHONY: build clean push create destroy test-e2e build-py build-py36

build: build-py build-py36

build-py: $(DOCKERFILE_CONTEXT)
	docker build -t $(LOCAL_IMAGE) $(DOCKERFILE_CONTEXT)

build-py36: $(DOCKERFILE_CONTEXT)-py36
	docker build -t $(LOCAL_IMAGE)-py36 $(DOCKERFILE_CONTEXT)-py36

clean: clean-context
	-docker rmi $(LOCAL_IMAGE)
	-docker rmi $(LOCAL_IMAGE)-py36

clean-target:
	-rm -rf target
	-rm -rf target-py36

clean-context:
	-rm -rf $(DOCKERFILE_CONTEXT)/*
	-rm -rf $(DOCKERFILE_CONTEXT)-py36/*

context: $(DOCKERFILE_CONTEXT) $(DOCKERFILE_CONTEXT)-py36

$(DOCKERFILE_CONTEXT): $(DOCKERFILE_CONTEXT)/Dockerfile $(DOCKERFILE_CONTEXT)/modules

$(DOCKERFILE_CONTEXT)-py36: $(DOCKERFILE_CONTEXT)-py36/Dockerfile $(DOCKERFILE_CONTEXT)-py36/modules

$(DOCKERFILE_CONTEXT)/Dockerfile $(DOCKERFILE_CONTEXT)/modules:
	-mkdir -p $(DOCKERFILE_CONTEXT)
	concreate generate --descriptor image-inc.yaml
	cp -R target/image/* $(DOCKERFILE_CONTEXT)

$(DOCKERFILE_CONTEXT)-py36/Dockerfile $(DOCKERFILE_CONTEXT)-py36/modules:
	-mkdir -p $(DOCKERFILE_CONTEXT)-py36
	concreate generate --descriptor image-inc.yaml --overrides overrides/python36.yaml --target target-py36
	cp -R target-py36/image/* $(DOCKERFILE_CONTEXT)-py36

zero-tarballs:
	find ./$(DOCKERFILE_CONTEXT) -name "*.tgz" -type f -exec truncate -s 0 {} \;
	find ./$(DOCKERFILE_CONTEXT) -name "*.tar.gz" -type f -exec truncate -s 0 {} \;
	find ./$(DOCKERFILE_CONTEXT)-py36 -name "*.tgz" -type f -exec truncate -s 0 {} \;
	find ./$(DOCKERFILE_CONTEXT)-py36 -name "*.tar.gz" -type f -exec truncate -s 0 {} \;

test-e2e:
	LOCAL_IMAGE=$(OPENSHIFT_SPARK_TEST_IMAGE) make -f Makefile.inc build
	test/run.sh incomplete/
	SPARK_TEST_IMAGE=$(OPENSHIFT_SPARK_TEST_IMAGE)-py36 test/run.sh incomplete/

test-e2e-py:
	LOCAL_IMAGE=$(OPENSHIFT_SPARK_TEST_IMAGE) make -f Makefile.inc build-py
	test/run.sh incomplete/

test-e2e-py36:
	LOCAL_IMAGE=$(OPENSHIFT_SPARK_TEST_IMAGE) make -f Makefile.inc build-py36
	SPARK_TEST_IMAGE=$(OPENSHIFT_SPARK_TEST_IMAGE)-py36 test/run.sh incomplete/