object "test" {
    code {
        let x := calldataload(4)
        mstore(32, 0xAA)

        let a := add(x, 32)
        x := add(x, 32)
        let b := x
        // This sstore cannot be eliminated.
        sstore(a, 32)
        let ret := sload(b)
        revert(ret, 32)
    }
}

// ----
// step: unusedStoreEliminatorNoSsaTransform
//
// {
//     let x := calldataload(4)
//     mstore(32, 0xAA)
//     let a := add(x, 32)
//     x := add(x, 32)
//     let b := x
//     sstore(a, 32)
//     let ret := sload(b)
//     revert(ret, 32)
// }
