contract test {
    mapping(uint8 => uint8) a;
    mapping(uint8 => uint8) b;
    function set_internal(mapping(uint8 => uint8) storage m, uint8 key, uint8 value) internal returns (uint8) {
        uint8 oldValue = m[key];
        m[key] = value;
        return oldValue;
    }
    function set(uint8 key, uint8 value_a, uint8 value_b) public returns (uint8 old_a, uint8 old_b) {
        old_a = set_internal(a, key, value_a);
        old_b = set_internal(b, key, value_b);
    }
    function get(uint8 key) public returns (uint8, uint8) {
        return (a[key], b[key]);
    }
}
// ----
// set(uint8,uint8,uint8): 1, 21, 42 -> 0, 0
// get(uint8): 1 -> 21, 42
// set(uint8,uint8,uint8): 1, 10, 11 -> 21, 42
// get(uint8): 1 -> 10, 11
