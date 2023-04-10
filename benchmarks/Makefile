clean:
	rm -f *.pbf *.pbf.json *linted*

JSON_PATH ?= ak_alaska_zip_codes_geo.min.json
ak_alaska_zip_codes_geo.min.json:
	wget https://raw.githubusercontent.com/OpenDataDE/State-zip-code-GeoJSON/master/ak_alaska_zip_codes_geo.min.json
pull: ak_alaska_zip_codes_geo.min.json
.PHONY: pull

roundtrip_js:
	make encoding_js
	make decoding_js
encoding_js:
	json2geobuf < $(JSON_PATH) > $(JSON_PATH)__js.pbf
decoding_js:
	geobuf2json < $(JSON_PATH)__js.pbf > $(JSON_PATH)__js.pbf.json
benchmark_js:
	hyperfine --warmup 3 'make encoding_js'
	hyperfine --warmup 3 'make decoding_js'
#    hyperfine --warmup 3 'make roundtrip_js'
.PHONY: roundtrip_js encoding_js decoding_js benchmark_js

roundtrip_py:
	make encoding_py
	make decoding_py
encoding_py:
	geobuf encode < $(JSON_PATH) > $(JSON_PATH)__py.pbf
decoding_py:
	geobuf decode < $(JSON_PATH)__py.pbf > $(JSON_PATH)__py.pbf.json
benchmark_py:
	hyperfine --warmup 3 'make encoding_py'
	hyperfine --warmup 3 'make decoding_py'
#    hyperfine --warmup 3 'make roundtrip_py'
.PHONY: roundtrip_py encoding_py decoding_py benchmark_py

roundtrip_cpp:
	make encoding_cpp
	make decoding_cpp
encoding_cpp:
	python3 -m pybind11_geobuf json2geobuf --precision=6 --only_xy=True $(JSON_PATH) $(JSON_PATH)__cpp.pbf
decoding_cpp:
	python3 -m pybind11_geobuf geobuf2json $(JSON_PATH)__cpp.pbf $(JSON_PATH)__cpp.pbf.json
benchmark_cpp:
	hyperfine --warmup 3 'make encoding_cpp'
	hyperfine --warmup 3 'make decoding_cpp'
#    hyperfine --warmup 3 'make roundtrip_cpp'
.PHONY: roundtrip_cpp encoding_cpp decoding_cpp benchmark_cpp

benchmark_all:
	make benchmark_js
	make benchmark_py
	make benchmark_cpp

normalize:
	python3 -m pybind11_geobuf normalize_json $(JSON_PATH) --output_path linted_input.json
	python3 -m pybind11_geobuf normalize_json $(JSON_PATH)__js.pbf.json --output_path linted_js.json
	python3 -m pybind11_geobuf normalize_json $(JSON_PATH)__py.pbf.json --output_path linted_py.json
	python3 -m pybind11_geobuf normalize_json $(JSON_PATH)__cpp.pbf.json --output_path linted_cpp.json

diff:
	diff linted_input.json linted_js.json
	diff linted_input.json linted_py.json
	diff linted_input.json linted_cpp.json

check:
	vim -p linted_input.json linted_js.json linted_py.json linted_cpp.json
