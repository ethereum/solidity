contract C {
    function g() public returns (uint ret) {
        uint x = type(uint).max;
        assembly {
            mstore(0x20, x)
            // both old and new optimizer should be able to evaluate this
            ret := keccak256(0x20, 16)
        }
    }

    function f() public returns (uint ret) {
        uint x = type(uint).max;
        assembly {
            mstore(0x20, x)
            // For Yul optimizer, load resolver and loop invariant code motion
            // would take the Keccak-256 outside the loop. For the old-optimizer,
            // this is not possible.
            // Net savings approximately: 20 * cost of Keccak-256 = 572
            for {let i := 0} lt(i, 20) { i := add(i, 1) } {
                ret := keccak256(0x20, 16)
            }
        }
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 0xcdb56c384a9682c600315e3470157a4cf7638d0d33e9dae5c40ffd2644fc5a80
// gas irOptimized: 22239
// gas legacy: 23385
// gas legacyOptimized: 23092
// g() -> 0xcdb56c384a9682c600315e3470157a4cf7638d0d33e9dae5c40ffd2644fc5a80
// gas irOptimized: 21277
// gas legacy: 21462
// gas legacyOptimized: 21256
