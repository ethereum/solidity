contract C {
    function f(string calldata x) external returns (bytes memory r) {
        uint mptr;
        assembly {
            // dirty memory
            mptr := mload(0x40)
            for { let i := mptr } lt(i, add(mptr, 0x0100)) { i := add(i, 32) }
            {
                mstore(i, sub(0, 1))
            }
        }
        r = abi.encode(x);
        assembly {
            // assert that we dirtied the memory that was encoded to
            if iszero(eq(mptr, r)) {
                revert(0, 0)
            }
        }
    }
    function test() external returns (bytes memory) {
        return this.f("abc");
    }
}
// ====
// EVMVersion: >homestead
// ----
// test() -> 0x20, 0x60, 0x20, 3, "abc"
