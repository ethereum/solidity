contract C {
    uint value;

    function set(uint _value) external {
        value = _value;
    }

    function get() external view returns(uint) {
        return value;
    }

    function get_delegated() external returns(bool, bytes memory) {
        return address(this).delegatecall(abi.encodeWithSignature("get()"));
    }

    function assert0() external view {
        assert(value == 0);
    }

    function assert0_delegated() external returns(bool, bytes memory) {
        return address(this).delegatecall(abi.encodeWithSignature("assert0()"));
    }
}

// ----
// get() -> 0
// get():"" -> "0"
// assert0_delegated() -> 1, 0x40, 0x00
// assert0_delegated():"" -> "1, 64, 0"
// get_delegated() -> 1, 0x40, 0x20, 0x00
// get_delegated():"" -> "1, 64, 32, 0"
// set(uint256): 1) -> 
// set(uint256):"1" -> ""
// get() -> 1
// get():"" -> "1"
// assert0_delegated() -> 0, 0x40, 0x00
// assert0_delegated():"" -> "0, 64, 0"
// get_delegated() -> 1, 0x40, 0x20, 1
// get_delegated():"" -> "1, 64, 32, 1"
// set(uint256): 42) -> 
// set(uint256):"42" -> ""
// get() -> 42
// get():"" -> "42"
// assert0_delegated() -> 0, 0x40, 0x00
// assert0_delegated():"" -> "0, 64, 0"
// get_delegated() -> 1, 0x40, 0x20, 42
// get_delegated():"" -> "1, 64, 32, 42"
