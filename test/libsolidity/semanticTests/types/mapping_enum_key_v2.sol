pragma abicoder v2;
enum E { A, B, C }
contract test {
    mapping(E => uint8) table;
    function get(E k) public returns (uint8 v) {
        return table[k];
    }
    function set(E k, uint8 v) public {
        table[k] = v;
    }
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: also
// ----
// get(uint8): 0 -> 0
// get(uint8): 0x01 -> 0
// get(uint8): 0x02 -> 0
// get(uint8): 0x03 -> FAILURE
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
