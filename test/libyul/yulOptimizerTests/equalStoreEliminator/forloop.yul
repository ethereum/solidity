{
    let x := calldataload(0)
    let y := calldataload(1)

    sstore(x, y)
    for {let a := 1} lt(a, 10) {a := add(a, 1) } {
        sstore(x, y)
    }
}
// ----
// step: equalStoreEliminator
//
// {
//     let x := calldataload(0)
//     let y := calldataload(1)
//     sstore(x, y)
//     let a := 1
//     for { } lt(a, 10) { a := add(a, 1) }
//     { sstore(x, y) }
// }
