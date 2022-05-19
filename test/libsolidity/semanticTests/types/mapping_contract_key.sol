interface A {}
contract test {
    mapping(A => uint8) table;
    function get(A k) public returns (uint8 v) {
        return table[k];
    }
    function set(A k, uint8 v) public {
        table[k] = v;
    }
}
// ----
// get(address): 0 -> 0
// get(address): 0x01 -> 0
// get(address): 0xa7 -> 0
// set(address,uint8): 0x01, 0xa1 ->
// get(address): 0 -> 0
// get(address): 0x01 -> 0xa1
// get(address): 0xa7 -> 0
// set(address,uint8): 0x00, 0xef ->
// get(address): 0 -> 0xef
// get(address): 0x01 -> 0xa1
// get(address): 0xa7 -> 0
// set(address,uint8): 0x01, 0x05 ->
// get(address): 0 -> 0xef
// get(address): 0x01 -> 0x05
// get(address): 0xa7 -> 0
