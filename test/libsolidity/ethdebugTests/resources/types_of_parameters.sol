struct S { uint256 a; }

function free(S memory s, uint8[] calldata xs) pure returns (bytes memory) {
    return abi.encode(s.a, xs.length);
}

library L {
    function internalFn(int64 v) internal pure returns (int64) { return v; }
    function externalFn(uint256[2] memory) external pure {}
}

contract Base {
    modifier m(address who) { require(who != address(0)); _; }
    function base(bool b) internal pure returns (bool) { return !b; }
}

contract C is Base {
    function f(S calldata s, string memory t) external pure returns (uint32[] memory) {
        return new uint32[](s.a + bytes(t).length);
    }
}
// ----
// .resources.types | keys: ["t_address","t_array$_t_uint256_$2_storage","t_array$_t_uint32_$dyn_storage","t_array$_t_uint8_$dyn_storage","t_bool","t_bytes_storage","t_int64","t_string_storage","t_struct$_S_$4_storage","t_uint256","t_uint32","t_uint8"]
// .resources.types.t_address: {"kind":"address","payable":false}
// .resources.types.t_array$_t_uint256_$2_storage: {
//     "contains": {
//         "type": {
//             "id": "t_uint256"
//         }
//     },
//     "count": "0x02",
//     "kind": "array"
// }
// .resources.types.t_array$_t_uint32_$dyn_storage: {
//     "contains": {
//         "type": {
//             "id": "t_uint32"
//         }
//     },
//     "kind": "array"
// }
// .resources.types.t_array$_t_uint8_$dyn_storage: {
//     "contains": {
//         "type": {
//             "id": "t_uint8"
//         }
//     },
//     "kind": "array"
// }
// .resources.types.t_bool: {"kind":"bool"}
// .resources.types.t_bytes_storage: {"kind":"bytes"}
// .resources.types.t_int64: {"bits":64,"kind":"int"}
// .resources.types.t_string_storage: {"kind":"string"}
// .resources.types.t_struct$_S_$4_storage: {
//     "contains": [
//         {
//             "name": "a",
//             "type": {
//                 "id": "t_uint256"
//             }
//         }
//     ],
//     "definition": {
//         "location": {
//             "range": {
//                 "length": 23,
//                 "offset": 59
//             },
//             "source": {
//                 "id": 0
//             }
//         },
//         "name": "S"
//     },
//     "kind": "struct"
// }
// .resources.types.t_uint256: {"bits":256,"kind":"uint"}
// .resources.types.t_uint32: {"bits":32,"kind":"uint"}
// .resources.types.t_uint8: {"bits":8,"kind":"uint"}
// .resources.pointers | keys: []
