{
    let x := calldataload(0)
    x := mload(x)
    let y := mload(x)
    sstore(0, y)
}
// ----
// step: loadResolver
//
// {
//     let _1 := 0
//     let x := calldataload(_1)
//     x := mload(x)
//     sstore(_1, mload(x))
// }
