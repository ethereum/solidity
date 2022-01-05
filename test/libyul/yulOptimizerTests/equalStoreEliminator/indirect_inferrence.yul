{
    let x := calldataload(0)
    let y := sload(x)
    // both of these can be removed
    sstore(x, y)
    sstore(x, y)

    let a := x
    let b := mload(a)
    // both of these can be removed
    mstore(a, b)
    mstore(a, b)
}
// ====
// EVMVersion: >=byzantium
// ----
// step: equalStoreEliminator
//
// {
//     let x := calldataload(0)
//     let y := sload(x)
//     let a := x
//     let b := mload(a)
// }
