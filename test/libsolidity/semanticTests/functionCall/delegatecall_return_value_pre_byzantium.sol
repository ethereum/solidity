contract C {
    uint256 value;

    function set(uint256 _value) external {
        value = _value;
    }

    function get() external view returns (uint256) {
        return value;
    }

    function get_delegated() external returns (bool) {
        (bool success,) = address(this).delegatecall(abi.encodeWithSignature("get()"));
        return success;
    }

    function assert0() external view {
        assert(value == 0);
    }

    function assert0_delegated() external returns (bool) {
        (bool success,) = address(this).delegatecall(abi.encodeWithSignature("assert0()"));
        return success;
    }
}
// ====
// compileViaYul: also
// EVMVersion: <byzantium
// ----
// get() -> 0x00
// assert0_delegated() -> true
// get_delegated() -> true
// set(uint256): 0x01 ->
// get() -> 0x01
// assert0_delegated() -> false
// get_delegated() -> true
// set(uint256): 0x2a ->
// get() -> 0x2a
// assert0_delegated() -> false
// get_delegated() -> true
