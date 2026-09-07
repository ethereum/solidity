// mstore(a, ...) must NOT be eliminated: it is read back via b one iteration later.
{
    let ptr := calldataload(0)
    let r := 0
    for { let i := 0 } lt(i, 3) { i := add(i, 1) } {
        let b := ptr // reads word written by previous iteration
        r := add(r, mload(b))
        let a := add(ptr, 0x20) // writes word read by next iteration
        mstore(a, add(i, 1))
        ptr := add(ptr, 0x20)
    }
    sstore(0, r)
}
// ====
// EVMVersion: >homestead
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let ptr := calldataload(0)
//         let ptr_9 := ptr
//         let r := 0
//         let r_10 := r
//         let i := 0
//         let i_11 := i
//         for { }
//         lt(i, 3)
//         {
//             let r_18 := r
//             let i_19 := i
//             let ptr_20 := ptr
//             i := add(i_19, 1)
//             let i_12 := i
//         }
//         {
//             let r_15 := r
//             let i_16 := i
//             let ptr_17 := ptr
//             r := add(r_15, mload(ptr_17))
//             let r_13 := r
//             let a := add(ptr_17, 0x20)
//             mstore(a, add(i_16, 1))
//             ptr := add(ptr_17, 0x20)
//             let ptr_14 := ptr
//         }
//         let r_21 := r
//         let i_22 := i
//         let ptr_23 := ptr
//         sstore(0, r_21)
//     }
// }
