clean:
	rm *.pbf *.pbf.json

JSON_PATH ?= ak_alaska_zip_codes_geo.min.json
ak_alaska_zip_codes_geo.min.json:
	wget https://raw.githubusercontent.com/OpenDataDE/State-zip-code-GeoJSON/master/ak_alaska_zip_codes_geo.min.json
pull: ak_alaska_zip_codes_geo.min.json
.PHONY: pull

roundtrip_js:
	make encoding_js
	make decoding_js
encoding_js:
	cat $(JSON_PATH) | json2geobuf > $(JSON_PATH)__js.pbf
decoding_js:
	cat $(JSON_PATH)__js.pbf | geobuf2json > $(JSON_PATH)__js.pbf.json
.PHONY: roundtrip_js encoding_js decoding_js

benchmark_js:
	hyperfine --warmup 3 'make encoding_js'
	hyperfine --warmup 3 'make decoding_js'
	hyperfine --warmup 3 'make roundtrip_js'

roundtrip_cpp:
	make encoding_cpp
	make decoding_cpp
encoding_cpp:
	python3 -m pybind11_geobuf json2geobuf --precision=6 --only_xy=True $(JSON_PATH) $(JSON_PATH)__cpp.pbf
decoding_cpp:
	python3 -m pybind11_geobuf geobuf2json $(JSON_PATH)__cpp.pbf $(JSON_PATH)__cpp.pbf.json
.PHONY: roundtrip_cpp encoding_cpp decoding_cpp

benchmark_cpp:
	hyperfine --warmup 3 'make encoding_cpp'
	hyperfine --warmup 3 'make decoding_cpp'
	hyperfine --warmup 3 'make roundtrip_cpp'
