{
    let x := calldataload(0)
    let a := mload(x)
    let b := mload(x)
    sstore(a, b)
}
// ----
// step: loadResolver
//
// {
//     {
//         let a := mload(calldataload(0))
//         sstore(a, a)
//     }
// }
