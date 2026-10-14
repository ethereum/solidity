contract C {
    function f(uint, uint) public returns (uint256 result) {
        assembly {
            let v := 0x400
            if calldataload(4) { v := 0 }
            let a := add(v, 0x100)
            mstore(a, 67)
            if calldataload(36) { v := 0x300 }
            let b := add(v, 0x200)
            mstore(b, 7)
            result := mload(a)
        }
    }
}
// ----
// f(uint256,uint256): 0, 0 -> 67
// f(uint256,uint256): 0, 1 -> 7
