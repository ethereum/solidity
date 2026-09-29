{
    // both opcodes are zero for this creation call, but opaque to the optimizer.
    sstore(0, add(0x400, calldatasize()))
    sstore(1, add(0x300, callvalue()))

    let dummyPosition := 0
    let dummyValue := 99
    mstore(dummyPosition, dummyValue) // prime the symbolic-offset cache

    let v := sload(0)
    let a1 := add(v, 0x100)
    let a := add(a1, 0x10)
    let oldValue := 67
    mstore(a, oldValue)

    v := sload(1)
    let b := add(add(v, 0x200), 0x10)
    let newValue := 7
    mstore(b, newValue)

    // the second store overwrote the first, so this load must return 7.
    // LoadResolver incorrectly replaces it with oldValue (67).
    let result := mload(a)
    mstore(0, result)
    return(0, 0x20)
}
// ----
// step: loadResolverSolo
//
// {
//     sstore(0, add(0x400, calldatasize()))
//     sstore(1, add(0x300, callvalue()))
//     let dummyPosition := 0
//     let dummyValue := 99
//     mstore(dummyPosition, dummyValue)
//     let v := sload(0)
//     let a1 := add(v, 0x100)
//     let a := add(a1, 0x10)
//     let oldValue := 67
//     mstore(a, oldValue)
//     v := sload(1)
//     let b := add(add(v, 0x200), 0x10)
//     let newValue := 7
//     mstore(b, newValue)
//     let result := mload(a)
//     mstore(0, result)
//     return(0, 0x20)
// }
