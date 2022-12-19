{
    let x := calldataload(0)
    let y := calldataload(1)
    let a := keccak256(x, y)
    sstore(a, 2)
    // reassign value
    a := calldataload(10)
    let b := keccak256(x, y)
    sstore(b, 3)
    // reassign arg1
    x := 10
    let c := keccak256(x, y)
    sstore(c, 4)
    // reassign arg2
    y := 9
    let d := keccak256(x, y)
    sstore(d, 5)
    // no reassign, check that it is still working here.
    let e := keccak256(x, y)
    sstore(e, 6)
}
// ----
// step: loadResolver
//
// {
//     {
//         let x := calldataload(0)
//         let y := calldataload(1)
//         let a := keccak256(x, y)
//         sstore(a, 2)
//         let _4 := 10
//         a := calldataload(_4)
//         sstore(keccak256(x, y), 3)
//         x := _4
//         sstore(keccak256(_4, y), 4)
//         y := 9
//         let d := keccak256(_4, y)
//         sstore(d, 5)
//         sstore(d, 6)
//     }
// }
