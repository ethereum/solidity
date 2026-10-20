{
    let a := calldataload(0)
    let p := add(a, 32)
    let q := add(a, 64)
    let oldValue := 7
    let newValue := 8
    mstore(q, oldValue)
    // The new value of `p` references `p` itself, so it is not tracked.
    p := add(p, 32)
    // `p` and `q` are the same location.
    mstore(p, newValue)
    sstore(0, mload(q))
}
// ----
// step: loadResolverSolo
//
// {
//     let a := calldataload(0)
//     let p := add(a, 32)
//     let q := add(a, 64)
//     let oldValue := 7
//     let newValue := 8
//     mstore(q, oldValue)
//     p := add(p, 32)
//     mstore(p, newValue)
//     sstore(0, mload(q))
// }
