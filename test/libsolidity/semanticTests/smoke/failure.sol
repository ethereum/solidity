contract C {
    function e() public pure {
        revert("Transaction failed.");
    }
    function f(bool _value) public pure {
        string memory message;
        require(_value, message);
    }
    function g(bool _value) public pure {
        require(_value, "Value is false.");
    }
    function h() public pure returns (uint) {
        assert(false);
    }
}
// ====
// EVMVersion: >homestead
// ----
// _() -> FAILURE
// e() -> FAILURE, hex"08c379a0", 0x20, 19, "Transaction failed."
// f(bool): false -> FAILURE, hex"08c379a0", 0x20, 0
// g(bool): false -> FAILURE, hex"08c379a0", 0x20, 15, "Value is false."
// h() -> FAILURE