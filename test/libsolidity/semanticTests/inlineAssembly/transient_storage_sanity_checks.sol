contract C {
    function f() external {
        assembly {
            tstore(0, 21)
            mstore(0, 42)
            sstore(0, 42)
            if iszero(eq(tload(0), 21)) {
                revert(0, 0)
            }
        }
    }
    function g() external view returns (uint s, uint m, uint t) {
        assembly {
            s := sload(0)
            m := mload(0)
            t := tload(0)
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// f() ->
// g() -> 0x2a, 0, 0
