interface A {}
contract test {
    mapping(A => uint8) public table;
    function set(A k, uint8 v) public {
        table[k] = v;
    }
    function get(A k) public returns (uint8) {
        return this.table(k);
    }
}
// ====
// compileViaYul: also
// ----
// table(address): 0 -> 0
// table(address): 0x01 -> 0
// table(address): 0xa7 -> 0
// get(address): 0 -> 0
// get(address): 0x01 -> 0
// get(address): 0xa7 -> 0
// set(address,uint8): 0x01, 0xa1 ->
// table(address): 0 -> 0
// table(address): 0x01 -> 0xa1
// table(address): 0xa7 -> 0
// get(address): 0 -> 0
// get(address): 0x01 -> 0xa1
// get(address): 0xa7 -> 0
// set(address,uint8): 0x00, 0xef ->
// table(address): 0 -> 0xef
// table(address): 0x01 -> 0xa1
// table(address): 0xa7 -> 0
// get(address): 0 -> 0xef
// get(address): 0x01 -> 0xa1
// get(address): 0xa7 -> 0
// set(address,uint8): 0x01, 0x05 ->
// table(address): 0 -> 0xef
// table(address): 0x01 -> 0x05
// table(address): 0xa7 -> 0
// get(address): 0 -> 0xef
// get(address): 0x01 -> 0x05
// get(address): 0xa7 -> 0
