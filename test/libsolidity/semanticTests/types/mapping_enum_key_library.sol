enum E { A, B, C }
library L {
    function get(mapping(E => uint8) storage table, E k) external returns (uint8) {
        return table[k];
    }
    function set(mapping(E => uint8) storage table, E k, uint8 v) external {
        table[k] = v;
    }
}
contract test {
    mapping(E => uint8) table;
    function get(E k) public returns (uint8 v) {
        return L.get(table, k);
    }
    function set(E k, uint8 v) public {
        L.set(table, k, v);
    }
}
// ----
// library: L
// get(uint8): 0 -> 0
// get(uint8): 0x01 -> 0
// get(uint8): 0xa7 -> FAILURE
// set(uint8,uint8): 0x01, 0xa1 ->
// get(uint8): 0 -> 0
// get(uint8): 0x01 -> 0xa1
// get(uint8): 0xa7 -> FAILURE
// set(uint8,uint8): 0x00, 0xef ->
// get(uint8): 0 -> 0xef
// get(uint8): 0x01 -> 0xa1
// get(uint8): 0xa7 -> FAILURE
// set(uint8,uint8): 0x01, 0x05 ->
// get(uint8): 0 -> 0xef
// get(uint8): 0x01 -> 0x05
// get(uint8): 0xa7 -> FAILURE
