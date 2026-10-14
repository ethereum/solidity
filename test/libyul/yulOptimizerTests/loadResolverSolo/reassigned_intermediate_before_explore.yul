{
    let a := calldataload(0)
    let b := add(a, 32)
    let c := add(b, 32)
    // Nothing has explored `c` yet. Evaluating it lazily now must not use the new value of `b`.
    b := a
    let e := add(a, 64)
    let oldValue := 7
    let newValue := 8
    // `c` and `e` are the same location.
    mstore(c, oldValue)
    mstore(e, newValue)
    sstore(0, mload(c))
}
// ----
// step: loadResolverSolo
//
// {
//     let a := calldataload(0)
//     let b := add(a, 32)
//     let c := add(b, 32)
//     b := a
//     let e := add(a, 64)
//     let oldValue := 7
//     let newValue := 8
//     mstore(c, oldValue)
//     mstore(e, newValue)
//     sstore(0, mload(c))
// }
