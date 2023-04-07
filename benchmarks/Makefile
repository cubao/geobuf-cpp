PROJECT_SOURCE_DIR ?= $(abspath ./)

ak_alaska_zip_codes_geo.min.json:
	wget https://raw.githubusercontent.com/OpenDataDE/State-zip-code-GeoJSON/master/ak_alaska_zip_codes_geo.min.json
clean:
	rm *.pbf *.pbf.json

JSON_PATH ?= ak_alaska_zip_codes_geo.min.json


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
