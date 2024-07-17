#!/usr/bin/env bash
set -euo pipefail

script_dir="$(realpath "$(dirname "$0")")"
soljson_js="$1"
soljson_wasm="$2"
soljson_wasm_size=$(wc -c "${soljson_wasm}" | cut -d ' ' -f 1)
output="$3"

(( $# == 3 )) || { >&2 echo "Usage: $0 soljson.js soljson.wasm packed_soljson.js"; exit 1; }

# If this changes in an emscripten update, it's probably nothing to worry about,
# but we should double-check when it happens and adjust the tail command below.
[[ $(head -c 5 "${soljson_js}") == "null;" ]] || { >&2 echo 'Expected soljson.js to start with "null;"'; exit 1; }

echo "Packing $soljson_js and $soljson_wasm to $output."
(
    echo -n 'var Module = Module || {}; Module["wasmBinary"] = '
    echo -n '(function(source, uncompressedSize) {'
    # Note that base64DecToArr assumes no trailing equals signs.
    cpp "${script_dir}/base64DecToArr.js" | grep -v "^#.*"
    # Note that mini-lz4.js assumes no file header and no frame crc checksums.
    cpp "${script_dir}/mini-lz4.js" | grep -v "^#.*"
    echo 'return uncompress(base64DecToArr(source), uncompressedSize);})('
    echo -n '"'
    # We fix lz4 format settings, remove the 8 bytes file header and remove the trailing equals signs of the base64 encoding.
    lz4c --no-frame-crc --best --favor-decSpeed "${soljson_wasm}" - | tail -c +8 | base64 -w 0 | sed 's/[^A-Za-z0-9\+\/]//g'
    echo '",'
    echo -n "${soljson_wasm_size});"
    # Remove "null;" from the js wrapper.
    tail -c +6 "${soljson_js}"
) > "$output"

echo "Testing $output."
echo "process.stdout.write(require('$(realpath "${output}")').wasmBinary)" | node | cmp "${soljson_wasm}" && echo "Binaries match."
# Allow the wasm binary to be garbage collected after compilation.
echo 'Module["wasmBinary"] = undefined;' >> "${output}"
