{
    let dummyPosition := 0
    let dummyValue := 99
    // Makes the knowledge base compute the offsets of `a` and `a1` relative to `v`.
    mstore(dummyPosition, dummyValue)

    let v := sload(0)
    let a1 := add(v, 0x100)
    let a := add(a1, 0x10)
    let oldValue := 67
    mstore(a, oldValue)

    v := sload(1)
    let b := add(add(v, 0x200), 0x10)
    let newValue := 7
    mstore(b, newValue)

    // `a` and `b` can be equal, so this cannot be replaced by the hash of `oldValue`.
    let len := 32
    let hash := keccak256(a, len)
    sstore(0, hash)
}
// ----
// step: loadResolverSolo
//
// {
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
//     let len := 32
//     let hash := keccak256(a, len)
//     sstore(0, hash)
// }
