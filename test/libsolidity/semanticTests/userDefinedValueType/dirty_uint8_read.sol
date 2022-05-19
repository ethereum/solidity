type MyInt8 is int8;
contract C {
    MyInt8 public x = MyInt8.wrap(-5);

    /// The most significant bit is flipped to 0
    function create_dirty_slot() external {
        uint mask  = 2**255 -1;
        assembly {
            let value := sload(x.slot)
            sstore(x.slot, and(mask, value))
        }
    }

    function read_unclean_value() external returns (bytes32 ret) {
        MyInt8 value = x;
        assembly {
            ret := value
        }
    }
}
// ----
// x() -> -5
// create_dirty_slot() ->
// read_unclean_value() -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffb
