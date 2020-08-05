contract C {
    uint256 value;

    function set(uint256 _value) external {
        value = _value;
    }

    function get() external view returns (uint256) {
        return value;
    }

    function get_delegated() external returns (bool, bytes memory) {
        return address(this).delegatecall(abi.encodeWithSignature("get()"));
    }

    function assert0() external view {
        assert(value == 0);
    }

    function assert0_delegated() external returns (bool, bytes memory) {
        return address(this).delegatecall(abi.encodeWithSignature("assert0()"));
    }
}
// ====
// compileViaYul: also
// EVMVersion: >=byzantium
// ----
// get() -> 0x00
// assert0_delegated() -> 0x01, 0x40, 0x0
// get_delegated() -> 0x01, 0x40, 0x20, 0x0
// set(uint256): 0x01 ->
// get() -> 0x01
// assert0_delegated() -> 0x00, 0x40, 0x0
// get_delegated() -> 0x01, 0x40, 0x20, 0x1
// set(uint256): 0x2a ->
// get() -> 0x2a
// assert0_delegated() -> 0x00, 0x40, 0x0
// get_delegated() -> 0x01, 0x40, 0x20, 0x2a
