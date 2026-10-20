{
    let a := calldataload(0)
    let x := a
    let y := 0
    let i := 0
    let oldValue := 7
    let newValue := 8
    for { } lt(i, 10) { i := add(i, 1) } {
        y := add(x, 32)
        let z := add(a, 64)
        // From the second iteration on, `y` and `z` are the same location.
        mstore(y, oldValue)
        mstore(z, newValue)
        sstore(i, mload(y))
        x := add(a, 32)
    }
}
// ----
// step: loadResolverSolo
//
// {
//     let a := calldataload(0)
//     let x := a
//     let y := 0
//     let i := 0
//     let oldValue := 7
//     let newValue := 8
//     for { } lt(i, 10) { i := add(i, 1) }
//     {
//         y := add(x, 32)
//         let z := add(a, 64)
//         mstore(y, oldValue)
//         mstore(z, newValue)
//         sstore(i, mload(y))
//         x := add(a, 32)
//     }
// }
