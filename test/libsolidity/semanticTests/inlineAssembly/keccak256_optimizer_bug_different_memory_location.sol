contract C {
    function f() public view returns (bool ret) {
        assembly {
            let x := calldataload(0)
            mstore(0, x)
            mstore(0x20, x)
            let a := keccak256(0, 4)
            let b := keccak256(0x20, 8)
            ret := eq(a, b)
        }
    }
}
// ====
// compileViaYul: also
// ----
// f() -> false
