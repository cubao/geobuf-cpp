PROJECT_SOURCE_DIR ?= $(abspath ./)
PROJECT_NAME ?= $(shell basename $(PROJECT_SOURCE_DIR))
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
	rm -rf $(BUILD_DIR) *.egg-info dist
force_clean:
	docker run --rm -v `pwd`:`pwd` -w `pwd` -it alpine/make make clean

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
pytest:
	python3 -m pip install pytest numpy
	pytest tests # --capture=tee-sys
.PHONY: test pytest

clean_test:
	rm -rf $(OUTPUT_DIR_JS) $(OUTPUT_DIR_CPP) build/roundtrip_test

docs_build:
	mkdocs build
docs_serve:
	mkdocs serve -a 0.0.0.0:8088

DOCKER_TAG_WINDOWS ?= ghcr.io/cubao/build-env-windows-x64:v0.0.1
DOCKER_TAG_LINUX ?= ghcr.io/cubao/build-env-manylinux2014-x64:v0.0.1
DOCKER_TAG_MACOS ?= ghcr.io/cubao/build-env-macos-arm64:v0.0.1

test_in_win:
	docker run --rm -w `pwd` -v `pwd`:`pwd` -v `pwd`/build/win:`pwd`/build -it $(DOCKER_TAG_WINDOWS) bash
test_in_mac:
	docker run --rm -w `pwd` -v `pwd`:`pwd` -v `pwd`/build/mac:`pwd`/build -it $(DOCKER_TAG_MACOS) bash
test_in_linux:
	docker run --rm -w `pwd` -v `pwd`:`pwd` -v `pwd`/build/linux:`pwd`/build -it $(DOCKER_TAG_LINUX) bash

PYTHON ?= python3
python_install:
	$(PYTHON) setup.py install --force
python_build:
	$(PYTHON) setup.py bdist_wheel
python_sdist:
	$(PYTHON) setup.py sdist
	# tar -tvf dist/geobuf-*.tar.gz
python_test: pytest

cli_test:
	python3 -m pybind11_geobuf
	python3 -m pybind11_geobuf --help
	python3 -m pybind11_geobuf json2geobuf data/sample1.json build/sample1.pbf
	python3 -m pybind11_geobuf geobuf2json build/sample1.pbf build/sample1.json --indent=True --sort_keys=True
	python3 -m pybind11_geobuf pbf_decode build/sample1.pbf build/sample1.pbf.txt
	python3 -m pybind11_geobuf normalize_json data/sample1.json build/sample1.normalized.json
	python3 -m pybind11_geobuf normalize_json data/sample1.json build/sample1.normalized.precision2.json --precision=2

# conda create -y -n py36 python=3.6
# conda create -y -n py37 python=3.7
# conda create -y -n py38 python=3.8
# conda create -y -n py39 python=3.9
# conda create -y -n py310 python=3.10
# conda env list
python_build_py36:
	PYTHON=python conda run --no-capture-output -n py36 make python_build
python_build_py37:
	PYTHON=python conda run --no-capture-output -n py37 make python_build
python_build_py38:
	PYTHON=python conda run --no-capture-output -n py38 make python_build
python_build_py39:
	PYTHON=python conda run --no-capture-output -n py39 make python_build
python_build_py310:
	PYTHON=python conda run --no-capture-output -n py310 make python_build
python_build_all: python_build_py36 python_build_py37 python_build_py38 python_build_py39 python_build_py310
python_build_all_in_linux:
	docker run --rm -w `pwd` -v `pwd`:`pwd` -v `pwd`/build/win:`pwd`/build -it $(DOCKER_TAG_LINUX) make python_build_all
	make repair_wheels && rm -rf dist/*.whl && mv wheelhouse/*.whl dist && rm -rf wheelhouse
python_build_all_in_macos: python_build_py38 python_build_py39 python_build_py310
python_build_all_in_windows: python_build_all

repair_wheels:
	python -m pip install auditwheel # sudo apt install patchelf
	ls dist/*-linux_x86_64.whl | xargs -n1 auditwheel repair --plat manylinux2014_x86_64
	rm -rf dist/*-linux_x86_64.whl && cp wheelhouse/*.whl dist && rm -rf wheelhouse

pypi_remote ?= pypi
upload_wheels:
	python -m pip install twine
	twine upload dist/*.whl -r $(pypi_remote)

tar.gz:
	tar -cvz --exclude .git -f ../$(PROJECT_NAME).tar.gz .
	ls -alh ../$(PROJECT_NAME).tar.gz

# https://stackoverflow.com/a/25817631
echo-%  : ; @echo -n $($*)
Echo-%  : ; @echo $($*)
ECHO-%  : ; @echo $* = $($*)
echo-Tab: ; @echo -n '    '
