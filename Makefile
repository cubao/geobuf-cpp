PROJECT_SOURCE_DIR ?= $(abspath ./)
BUILD_DIR ?= $(PROJECT_SOURCE_DIR)/build
INSTALL_DIR ?= $(BUILD_DIR)/install
NUM_JOB ?= 8

all:
	@echo nothing special

lint:
	pre-commit run -a
lint_install:
	pre-commit install
.PHONY: lint

reset_submodules:
	git submodule update --init --recursive

clean:
	rm -rf $(BUILD_DIR)

CMAKE_ARGS ?= \
	-DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) \
	-DBUILD_SHARED_LIBS=OFF
build:
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && \
	cmake $(PROJECT_SOURCE_DIR) $(CMAKE_ARGS) && \
	make -j$(NUM_JOB) && make install
.PHONY: build

test_all:
	@cd build && for t in $(wildcard $(BUILD_DIR)/bin/test_*); do echo $$t && eval $$t >/dev/null 2>&1 && echo 'ok' || echo $(RED)Not Ok$(NC); done

build_all:
	python3 -m mdk_tools.cli.run_in_build_env --docker-tag u16 make build
	python3 -m mdk_tools.cli.run_in_build_env --docker-tag u18 make build
	python3 -m mdk_tools.cli.run_in_build_env --docker-tag u20 make build
	python3 -m mdk_tools.cli.run_in_build_env --docker-tag win make build
	python3 -m mdk_tools.cli.run_in_build_env --docker-tag mph make build

INPUT_GEOJSON_PATH ?= data/sample1.json
# INPUT_GEOJSON_PATH := pygeobuf/test/fixtures/geometrycollection.json
GEOJSON_BASENAME = $(shell basename $(abspath $(INPUT_GEOJSON_PATH)))

OUTPUT_DIR_JS ?= $(BUILD_DIR)/js
OUTPUT_PBF_JS = $(OUTPUT_DIR_JS)/$(GEOJSON_BASENAME).pbf
OUTPUT_TXT_JS = $(OUTPUT_PBF_JS).txt
OUTPUT_JSN_JS = $(OUTPUT_PBF_JS).json

OUTPUT_DIR_CPP ?= $(BUILD_DIR)/cpp
OUTPUT_PBF_CPP = $(OUTPUT_DIR_CPP)/$(GEOJSON_BASENAME).pbf
OUTPUT_TXT_CPP = $(OUTPUT_PBF_CPP).txt
OUTPUT_JSN_CPP = $(OUTPUT_PBF_CPP).json

build/bin/json2geobuf: build

# LINTJSON := jq .
LINTJSON := $(BUILD_DIR)/bin/lintjson
roundtrip_test_js:
	@umask 0000 && mkdir -p $(OUTPUT_DIR_JS)
	json2geobuf $(INPUT_GEOJSON_PATH) > $(OUTPUT_PBF_JS)
	build/bin/pbf_decoder $(OUTPUT_PBF_JS) > $(OUTPUT_TXT_JS)
	geobuf2json $(OUTPUT_PBF_JS) | $(LINTJSON) > $(OUTPUT_JSN_JS)
	cat $(INPUT_GEOJSON_PATH) | $(LINTJSON) > $(OUTPUT_DIR_JS)/$(GEOJSON_BASENAME)
roundtrip_test_cpp: build/bin/json2geobuf
	@umask 0000 && mkdir -p $(OUTPUT_DIR_CPP)
	$(BUILD_DIR)/bin/json2geobuf $(INPUT_GEOJSON_PATH) > $(OUTPUT_PBF_CPP)
	build/bin/pbf_decoder $(OUTPUT_PBF_CPP) > $(OUTPUT_TXT_CPP)
	$(BUILD_DIR)/bin/geobuf2json $(OUTPUT_PBF_CPP) | $(LINTJSON) > $(OUTPUT_JSN_CPP)
	cat $(INPUT_GEOJSON_PATH) | $(LINTJSON) > $(OUTPUT_DIR_CPP)/$(GEOJSON_BASENAME)
roundtrip_test_cpp: build/bin/json2geobuf
diff:
	# code --diff $(OUTPUT_TXT_JS) $(OUTPUT_TXT_CPP)
	code --diff $(OUTPUT_JSN_JS) $(OUTPUT_JSN_CPP)

test:
	# make roundtrip_test_js roundtrip_test_cpp diff
	python3 geobuf-roundtrip-test.py pygeobuf/test/fixtures

clean_test:
	rm -rf $(OUTPUT_DIR_JS) $(OUTPUT_DIR_CPP) build/roundtrip_test
