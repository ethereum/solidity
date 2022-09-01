contract test {
    mapping(uint8 => uint8) table;
    function get(uint8 k) public returns (uint8 v) {
        return table[k];
    }
    function set(uint8 k, uint8 v) public {
        table[k] = v;
    }
}
// ----
// get(uint8): 0 -> 0
// get(uint8): 0x01 -> 0
// get(uint8): 0xa7 -> 0
// set(uint8,uint8): 0x01, 0xa1 ->
// get(uint8): 0 -> 0
// get(uint8): 0x01 -> 0xa1
// get(uint8): 0xa7 -> 0
// set(uint8,uint8): 0x00, 0xef ->
// get(uint8): 0 -> 0xef
// get(uint8): 0x01 -> 0xa1
// get(uint8): 0xa7 -> 0
// set(uint8,uint8): 0x01, 0x05 ->
// get(uint8): 0 -> 0xef
// get(uint8): 0x01 -> 0x05
// get(uint8): 0xa7 -> 0
