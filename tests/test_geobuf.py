import json

import pybind11_geobuf
from pybind11_geobuf import Decoder, Encoder

print(pybind11_geobuf.__version__)

encoder = Encoder(max_precision=int(1e8))

geojson = {
    "type": "Feature",
    "properties": {},
    "geometry": {
        "coordinates": [
            [120.40317479950272, 31.416966084052177],
            [120.28451900911591, 31.30578266928819],
            [120.35592249359615, 31.21781895672254],
            [120.67093786630113, 31.299502266522722],
        ],
        "type": "LineString",
    },
}

encoded = encoder.encode(geojson=json.dumps(geojson))

decoder = Decoder()
geojson_text = decoder.decode(encoded, indent=True)
print(geojson_text)
