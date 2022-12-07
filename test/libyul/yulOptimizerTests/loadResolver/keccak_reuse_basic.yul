{
    let x := calldataload(0)
    let a := keccak256(0, x)
    sstore(a, 2)
    let t := mload(2)
    let b := keccak256(0, x)
    sstore(b, 3)
}
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 0
//         let a := keccak256(_1, calldataload(_1))
//         sstore(a, 2)
//         sstore(a, 3)
//     }
// }
