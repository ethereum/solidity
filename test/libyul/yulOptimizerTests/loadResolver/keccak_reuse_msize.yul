{
    let a := calldataload(0)
    let t := msize()
    let x := keccak256(0, a)
    let y := keccak256(0, a)
    sstore(x, y)
}
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 0
//         let a := calldataload(_1)
//         let x := keccak256(_1, a)
//         sstore(x, keccak256(_1, a))
//     }
// }
