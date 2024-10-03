PROJECT_SOURCE_DIR ?= $(abspath ./)
PROJECT_NAME ?= $(shell basename $(PROJECT_SOURCE_DIR))
BUILD_DIR ?= $(PROJECT_SOURCE_DIR)/build
INSTALL_DIR ?= $(BUILD_DIR)/install
NUM_JOBS ?= 8

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
	rm -rf $(BUILD_DIR) *.egg-info dist stubs
force_clean:
	docker run --rm -v `pwd`:`pwd` -w `pwd` -it alpine/make make clean

PYTHON ?= python3
build:
	$(PYTHON) -m pip install scikit_build_core pyproject_metadata pathspec pybind11
	CMAKE_BUILD_PARALLEL_LEVEL=$(NUM_JOBS) $(PYTHON) -m pip install --no-build-isolation -Ceditable.rebuild=true -Cbuild-dir=build -ve.
python_install:
	$(PYTHON) -m pip install . --verbose
python_wheel:
	$(PYTHON) -m pip wheel . -w build --verbose
python_sdist:
	$(PYTHON) -m pip sdist . --verbose
python_test: pytest
test:
	# make roundtrip_test_js roundtrip_test_cpp diff
	python3 geobuf-roundtrip-test.py pygeobuf/test/fixtures
pytest:
	python3 -m pip install pytest numpy
	pytest tests/test_basic.py
cli_test: cli_test1 cli_test2 cli_test3 cli_test4
.PHONY: build python_install python_wheel python_sdist test pytest cli_test

restub:
	pybind11-stubgen pybind11_geobuf._core -o stubs
	cp -rf stubs/pybind11_geobuf/_core src/pybind11_geobuf

test_all:
	@cd build && for t in $(wildcard $(BUILD_DIR)/test_*); do echo $$t && eval $$t >/dev/null 2>&1 && echo 'ok' || echo $(RED)Not Ok$(NC); done

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

build/json2geobuf: build

# LINTJSON := jq .
LINTJSON := $(BUILD_DIR)/lintjson
roundtrip_test_js:
	@umask 0000 && mkdir -p $(OUTPUT_DIR_JS)
	json2geobuf $(INPUT_GEOJSON_PATH) > $(OUTPUT_PBF_JS)
	build/pbf_decoder $(OUTPUT_PBF_JS) > $(OUTPUT_TXT_JS)
	geobuf2json $(OUTPUT_PBF_JS) | $(LINTJSON) > $(OUTPUT_JSN_JS)
	cat $(INPUT_GEOJSON_PATH) | $(LINTJSON) > $(OUTPUT_DIR_JS)/$(GEOJSON_BASENAME)
roundtrip_test_cpp: build/json2geobuf
	@umask 0000 && mkdir -p $(OUTPUT_DIR_CPP)
	$(BUILD_DIR)/json2geobuf $(INPUT_GEOJSON_PATH) > $(OUTPUT_PBF_CPP)
	build/pbf_decoder $(OUTPUT_PBF_CPP) > $(OUTPUT_TXT_CPP)
	$(BUILD_DIR)/geobuf2json $(OUTPUT_PBF_CPP) | $(LINTJSON) > $(OUTPUT_JSN_CPP)
	cat $(INPUT_GEOJSON_PATH) | $(LINTJSON) > $(OUTPUT_DIR_CPP)/$(GEOJSON_BASENAME)
diff:
	# code --diff $(OUTPUT_TXT_JS) $(OUTPUT_TXT_CPP)
	code --diff $(OUTPUT_JSN_JS) $(OUTPUT_JSN_CPP)


clean_test:
	rm -rf $(OUTPUT_DIR_JS) $(OUTPUT_DIR_CPP) build/roundtrip_test

docs_build:
	mkdocs build
docs_serve:
	mkdocs serve -a 0.0.0.0:8088

DOCKER_TAG_WINDOWS ?= ghcr.io/cubao/build-env-windows-x64:v0.0.1
DOCKER_TAG_LINUX ?= ghcr.io/cubao/build-env-manylinux2014-x64:v0.0.4
DOCKER_TAG_MACOS ?= ghcr.io/cubao/build-env-macos-arm64:v0.0.1

test_in_win:
	docker run --rm -w `pwd` -v `pwd`:`pwd` -v `pwd`/build/win:`pwd`/build -it $(DOCKER_TAG_WINDOWS) bash
test_in_mac:
	docker run --rm -w `pwd` -v `pwd`:`pwd` -v `pwd`/build/mac:`pwd`/build -it $(DOCKER_TAG_MACOS) bash
test_in_linux:
	docker run --rm -w `pwd` -v `pwd`:`pwd` -v `pwd`/build/linux:`pwd`/build -it $(DOCKER_TAG_LINUX) bash

DEV_CONTAINER_NAME ?= $(USER)_$(subst /,_,$(PROJECT_NAME)____$(PROJECT_SOURCE_DIR))
DEV_CONTAINER_IMAG ?= $(DOCKER_TAG_LINUX)
test_in_dev_container:
	docker ps | grep $(DEV_CONTAINER_NAME) \
		&& docker exec -it $(DEV_CONTAINER_NAME) bash \
		|| docker run --rm --name $(DEV_CONTAINER_NAME) \
			--network host --security-opt seccomp=unconfined \
			-v `pwd`:`pwd` -w `pwd` -it $(DEV_CONTAINER_IMAG) bash

cli_test1:
	python3 -m pybind11_geobuf
	python3 -m pybind11_geobuf --help
	python3 -m pybind11_geobuf json2geobuf data/sample1.json build/sample1.pbf
	python3 -m pybind11_geobuf geobuf2json build/sample1.pbf build/sample1.json --indent=True --sort_keys=True
	python3 -m pybind11_geobuf pbf_decode build/sample1.pbf build/sample1.pbf.txt
	python3 -m pybind11_geobuf normalize_json data/sample1.json build/sample1.normalized.json
	python3 -m pybind11_geobuf normalize_json data/sample1.json build/sample1.normalized.precision2.json --precision=2

cli_test2:
	python3 -m pybind11_geobuf round_trip data/sample2.json -o build/test/pybind --json2pb_use_python=False --pb2json_use_python=False
	python3 -m pybind11_geobuf round_trip data/sample2.json -o build/test/python --json2pb_use_python=True --pb2json_use_python=True
	python3 -m pybind11_geobuf round_trip data/sample2.json -o build/test/cxx_py --json2pb_use_python=False --pb2json_use_python=True
	python3 -m pybind11_geobuf round_trip data/sample2.json -o build/test/py_cxx --json2pb_use_python=True --pb2json_use_python=False

cli_test3:
	python3 -m pybind11_geobuf normalize_json data/feature_collection.json build/fc.json && cat build/fc.json | grep '"double": 3.142,'
	python3 -m pybind11_geobuf normalize_json data/feature_collection.json build/fc.json --round_geojson_non_geometry=None && cat build/fc.json | grep '"double": 3.141592653'
	python3 -m pybind11_geobuf normalize_json data/feature_collection.json build/fc.json --round_geojson_geometry=None && cat build/fc.json | grep 3.3333333333333
	python3 -m pybind11_geobuf normalize_json data/feature_collection.json build/fc.json --round_geojson_geometry=3,3,2 && cat build/fc.json | grep '120.285,'
	python3 -m pybind11_geobuf normalize_json data/feature_collection.json build/fc.json --round_geojson_geometry=0,0,0 && cat build/fc.json | grep '120,'
	python3 -m pybind11_geobuf normalize_json data/feature_collection.json build/fc.json --round_geojson_geometry=0,0,0 --denoise_double_0=False && cat build/fc.json | grep '120.0,'
	python3 -m pybind11_geobuf normalize_json data/feature_collection.json build/fc.json --round_geojson_geometry=8,8,-1 && cat build/fc.json | wc -l | grep 49
	python3 -m pybind11_geobuf normalize_json data/feature_collection.json build/fc.json --round_geojson_geometry=8,8,-1 --strip_geometry_z_0=False && cat build/fc.json | wc -l | grep 53

cli_test4:
	python3 -m pybind11_geobuf is_subset_of data/feature_collection.json data/feature_collection.json

.PHONY: cli_test1 cli_test2 cli_test3

geobuf_index.js: geobuf_index.proto
	pbf $< > $@

# conda create -y -n py38 python=3.8
# conda create -y -n py39 python=3.9
# conda create -y -n py310 python=3.10
# conda create -y -n py311 python=3.11
# conda create -y -n py312 python=3.12
# conda env list
python_build_py38:
	PYTHON=python conda run --no-capture-output -n py38 make python_build
python_build_py39:
	PYTHON=python conda run --no-capture-output -n py39 make python_build
python_build_py310:
	PYTHON=python conda run --no-capture-output -n py310 make python_build
python_build_py311:
	PYTHON=python conda run --no-capture-output -n py311 make python_build
python_build_py312:
	PYTHON=python conda run --no-capture-output -n py312 make python_build
python_build_all: python_build_py38 python_build_py39 python_build_py310 python_build_py311 python_build_py312
python_build_all_in_linux:
	docker run --rm -w `pwd` -v `pwd`:`pwd` -v `pwd`/build/linux:`pwd`/build -it $(DOCKER_TAG_LINUX) make python_build_all
	make repair_wheels && rm -rf dist/*.whl && mv wheelhouse/*.whl dist && rm -rf wheelhouse
python_build_all_in_macos: python_build_py38 python_build_py39 python_build_py310 python_build_py311 python_build_py312
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

SYNC_OUTPUT_DIR ?= headers/include/cubao
sync_headers:
	cp src/pybind11_rapidjson.cpp $(SYNC_OUTPUT_DIR)/pybind11_rapidjson.hpp
	cp src/geobuf/geojson_helpers.hpp $(SYNC_OUTPUT_DIR)
	cp src/geobuf/pybind11_helpers.hpp $(SYNC_OUTPUT_DIR)
	cp src/geobuf/rapidjson_helpers.hpp $(SYNC_OUTPUT_DIR)

# https://stackoverflow.com/a/25817631
echo-%  : ; @echo -n $($*)
Echo-%  : ; @echo $($*)
ECHO-%  : ; @echo $* = $($*)
echo-Tab: ; @echo -n '    '
