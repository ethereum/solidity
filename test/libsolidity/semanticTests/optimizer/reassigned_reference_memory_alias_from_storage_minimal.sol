contract C {
    uint s0 = 0x400;
    uint s1 = 0;
    uint s2 = 0x300;
    function f(uint, uint) public returns (uint256 result) {
        assembly {
            let v := sload(0)
            if calldataload(4) { v := sload(1) }
            let a := add(v, 0x100)
            mstore(a, 67)
            if calldataload(36) { v := sload(2) }
            let b := add(v, 0x200)
            mstore(b, 7)
            result := mload(a)
        }
    }
}
// ====
// compileViaYul: also
// ----
// f(uint256,uint256): 0, 0 -> 67
// f(uint256,uint256): 0, 1 -> 7
