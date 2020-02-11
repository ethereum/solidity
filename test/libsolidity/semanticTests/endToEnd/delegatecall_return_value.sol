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
// assert0_delegated() -> 1, 0x40, 0x00
// get_delegated() -> 1, 0x40, 0x20, 0x00
// set(uint256): 1 -> 
// get() -> 1
// assert0_delegated() -> 0, 0x40, 0x00
// get_delegated() -> 1, 0x40, 0x20, 1
// set(uint256): 42 -> 
// get() -> 42
// assert0_delegated() -> 0, 0x40, 0x00
// get_delegated() -> 1, 0x40, 0x20, 42
