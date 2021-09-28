type MyUInt16 is uint16;
type MyBytes2 is bytes2;
contract C {
    MyUInt16 public a = MyUInt16.wrap(13);
    MyBytes2 public b = MyBytes2.wrap(bytes2(uint16(1025)));
    bytes2 public x;
    function write_a() external {
        uint max = 0xf00e0bbc0d0d0d0d0d0d0d0d0d0d0d0d0d0d0d0d0d0e0c0ba098076054032001;
        assembly {
            sstore(a.slot, max)
        }
    }
    function write_b() external {
        uint max = 0xf00e0bbc0d0d0d0d0d0d0d0d0d0d0d0d0d0d0d0d0d0e0c0ba098076054032001;
        assembly {
            sstore(b.slot, max)
        }
    }
    function get_b(uint index) public returns (bytes1) {
        return MyBytes2.unwrap(b)[index];
    }
}
// ====
// compileViaYul: also
// ----
// a() -> 13
// b() -> 0x0401000000000000000000000000000000000000000000000000000000000000
// get_b(uint256): 0 -> 0x0400000000000000000000000000000000000000000000000000000000000000
// get_b(uint256): 1 -> 0x0100000000000000000000000000000000000000000000000000000000000000
// get_b(uint256): 2 -> FAILURE, hex"4e487b71", 0x32
// write_a() ->
// a() -> 0x2001
// write_b() ->
// b() -> 0x5403000000000000000000000000000000000000000000000000000000000000
// get_b(uint256): 0 -> 0x5400000000000000000000000000000000000000000000000000000000000000
// get_b(uint256): 1 -> 0x0300000000000000000000000000000000000000000000000000000000000000
// get_b(uint256): 2 -> FAILURE, hex"4e487b71", 0x32
