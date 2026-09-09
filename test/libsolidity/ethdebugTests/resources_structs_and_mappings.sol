struct Point { uint8 x; uint8 y; bytes4 salt; }
struct Line { Point from; Point to; string label; }

contract C {
    Point point;
    Line line;
    mapping(address => uint256) balances;
    mapping(address => mapping(uint256 => Line)) lines;
    mapping(string => bool) named;
}
// ----
// .resources.types | keys: <IGNORE>
// .resources.types.t_mapping$_t_address_$_t_uint256_$: {"kind": "mapping", "contains": {"key": {"type": {"id": "t_address"}}, "value": {"type": {"id": "t_uint256"}}}}
// .resources.types.t_mapping$_t_string_memory_ptr_$_t_bool_$.contains.key.type.id: t_string_memory_ptr
// .resources.pointers | length: 5
