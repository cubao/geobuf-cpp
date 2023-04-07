import sys
import json
with open(sys.argv[1], 'r', encoding='utf-8') as f:
    data = json.load(f)

def list_chop_z(data):
    if not isinstance(data, list):
        return
    if isinstance(data[0], float) and len(data) == 3 and data[2] == 0.0:
        data.pop()
    for d in data:
        list_chop_z(d)

for f in data['features']:
    coords = f['geometry']['coordinates']
    list_chop_z(coords)

with open(sys.argv[1], 'w', encoding='utf-8') as f:
    json.dump(data, f, indent=4)
