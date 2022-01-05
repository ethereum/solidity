{
    let var_k := calldataload(0)
    let _1 := 0x00
    let _2 := 0x20
    mstore(_1, var_k)
    mstore(_2, _1)
    sstore(keccak256(_1, 0x40), 0x01)
    mstore(_1, var_k)
    mstore(_2, _1)
    sstore(add(keccak256(_1, 0x40), 0x01), 0x03)
    mstore(_1, var_k)
    mstore(_2, _1)
    sstore(add(keccak256(_1, 0x40), 2), 0x04)
    mstore(_1, var_k)
    mstore(_2, _1)
    sstore(add(keccak256(_1, 0x40), 0x03), 2)
}
// ----
// step: equalStoreEliminator
//
// {
//     let var_k := calldataload(0)
//     let _1 := 0x00
//     let _2 := 0x20
//     mstore(_1, var_k)
//     mstore(_2, _1)
//     sstore(keccak256(_1, 0x40), 0x01)
//     sstore(add(keccak256(_1, 0x40), 0x01), 0x03)
//     sstore(add(keccak256(_1, 0x40), 2), 0x04)
//     sstore(add(keccak256(_1, 0x40), 0x03), 2)
// }
