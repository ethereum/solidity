{
    let a := calldataload(0)
    let t := msize()
    let x := keccak256(0, a)
    let y := keccak256(0, a)
    sstore(x, y)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         let a := calldataload(0)
//         let x := keccak256(0, a)
//         sstore(x, keccak256(0, a))
//     }
// }
