contract test {
    mapping(uint8 => uint8)[2] a;
    mapping(uint8 => uint8)[2] b;
    function set_internal(mapping(uint8 => uint8)[2] storage m, uint8 key, uint8 value1, uint8 value2) internal returns (uint8, uint8) {
        uint8 oldValue1 = m[0][key];
        uint8 oldValue2 = m[1][key];
        m[0][key] = value1;
        m[1][key] = value2;
        return (oldValue1, oldValue2);
    }
    function set(uint8 key, uint8 value_a1, uint8 value_a2, uint8 value_b1, uint8 value_b2) public returns (uint8 old_a1, uint8 old_a2, uint8 old_b1, uint8 old_b2) {
        (old_a1, old_a2) = set_internal(a, key, value_a1, value_a2);
        (old_b1, old_b2) = set_internal(b, key, value_b1, value_b2);
    }
    function get(uint8 key) public returns (uint8, uint8, uint8, uint8) {
        return (a[0][key], a[1][key], b[0][key], b[1][key]);
    }
}
// ----
// set(uint8,uint8,uint8,uint8,uint8): 1, 21, 22, 42, 43 -> 0, 0, 0, 0
// get(uint8): 1 -> 21, 22, 42, 43
// set(uint8,uint8,uint8,uint8,uint8): 1, 10, 30, 11, 31 -> 21, 22, 42, 43
// get(uint8): 1 -> 10, 30, 11, 31
