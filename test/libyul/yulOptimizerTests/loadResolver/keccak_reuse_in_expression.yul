{
    let x := calldataload(0)
    sstore(keccak256(0, x), keccak256(0, x))
}
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 0
//         let _3 := keccak256(_1, calldataload(_1))
//         sstore(_3, _3)
//     }
// }
