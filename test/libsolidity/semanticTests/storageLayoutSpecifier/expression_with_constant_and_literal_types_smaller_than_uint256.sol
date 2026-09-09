uint8 constant base = 255;
contract C layout at base + 257 {
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
