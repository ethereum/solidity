contract C {
    fallback() external {
        revert("abc");
    }

    function f() public returns (uint s, uint r) {
        address x = address(this);
        assembly {
            mstore(0, 7)
            s := call(sub(0, 1), x, 0, 0, 0, 0, 32)
            r := mload(0)
        }
    }
}

// ====
// compileViaYul: also
// EVMVersion: >=byzantium
// ----
// f() -> 0x00, 0x08c379a000000000000000000000000000000000000000000000000000000000
