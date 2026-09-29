{
    let v := sload(0)
    let a1 := add(v, 0x20)
    let a := a1
    let x := 67
    mstore(v, x)
    mstore(a, x)
    // The value of `v` is unknown both before and after this.
    switch calldataload(0) case 0 { v := 0x20 }
    let y := 7
    mstore(v, y)
    // `v` and `a` can be equal, so this cannot be resolved.
    sstore(0, mload(a))
}
// ----
// step: loadResolverSolo
//
// {
//     let v := sload(0)
//     let a1 := add(v, 0x20)
//     let a := a1
//     let x := 67
//     mstore(v, x)
//     mstore(a, x)
//     switch calldataload(0)
//     case 0 { v := 0x20 }
//     let y := 7
//     mstore(v, y)
//     sstore(0, x)
// }
