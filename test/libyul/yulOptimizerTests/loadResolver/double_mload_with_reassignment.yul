{
    let x := calldataload(0)
    let a := mload(x)
    a := 7
    let b := mload(x)
    sstore(a, b)
}
// ----
// step: loadResolver
//
// {
//     let x := calldataload(0)
//     let a := mload(x)
//     a := 7
//     sstore(a, mload(x))
// }
