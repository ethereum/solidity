{
    let a := 0
    let b := 1
    let c := 2
    mstore(a, b)
    sstore(0, mload(a))
    mstore(a, c)
    sstore(10, mload(a))
}
// ====
// step: loadResolver
// ----
// {
//     let a := 0
//     let b := 1
//     let c := 2
//     mstore(a, b)
//     sstore(a, b)
//     mstore(a, c)
//     sstore(10, c)
// }
