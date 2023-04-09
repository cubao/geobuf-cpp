#!/bin/bash

set -euo pipefail

file1="${1:?
Usage: $(basename $0) file1.json file2.json
Show diff of JSON, normalizing key ordering.
}"

file2="${2:?Missing file2}"

diff <(jq --sort-keys . < "$file1") <(jq --sort-keys . < "$file2") | less -R
