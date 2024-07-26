contract C {
    fallback() external {
        revert("abc");
    }

    function f() public returns (uint s, uint r) {
        address x = address(this);
        assembly {
            mstore(0, 7)
            s := extcall(x, 0, 0, 0)
            returndatacopy(0, 0, 32)
            r := mload(0)
        }
    }
}
// ====
// EVMVersion: >=byzantium
// compileToEOF: true
// EVMVersion: >=prague
// ----
// f() -> 0x01, 0x08c379a000000000000000000000000000000000000000000000000000000000
