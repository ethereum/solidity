uint16 constant base = 511;
uint8 constant offset = 1;
contract C layout at base + offset {
    uint public x = 7;
    function test() public returns (uint r) {
        assembly {
            r := sload(512)
            sstore(512, add(r, 1))
        }
    }
}
// ----
// test() -> 7
// x() -> 8
