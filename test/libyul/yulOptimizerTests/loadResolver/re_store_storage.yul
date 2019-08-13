{
    let a := 0
    let b := 1
    let c := 2
    sstore(a, b)
    mstore(0, sload(a))
    sstore(a, c)
    mstore(32, sload(a))
}
// ====
// step: loadResolver
// ----
// {
//     let a := 0
//     let b := 1
//     let c := 2
//     sstore(a, b)
//     mstore(a, b)
//     sstore(a, c)
//     mstore(32, c)
// }
