{
    let x := calldataload(0)
    let len := sload(x)
    let sum
    for { let i := 0} lt(i, sload(x)) { i := add(i, 1) } {
        let p := add(x, add(i, 1))
        if gt(p, sload(x)) { revert(0, 0) }
        sum := add(sum, sload(p))
    }
    mstore(0, sum)
    return(0, 0x20)
}
// ----
// step: loadResolver
//
// {
//     let _1 := 0
//     let x := calldataload(_1)
//     let len := sload(x)
//     let sum
//     let i := _1
//     for { } lt(i, len) { i := add(i, 1) }
//     {
//         let p := add(add(x, i), 1)
//         if gt(p, len) { revert(_1, _1) }
//         sum := add(sum, sload(p))
//     }
//     mstore(_1, sum)
//     return(_1, 0x20)
// }
