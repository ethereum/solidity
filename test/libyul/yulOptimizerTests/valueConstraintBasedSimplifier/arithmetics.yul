{
    let a := and(calldataload(0), 0xffff)
    let b := and(calldataload(1), 0xffff)
    let c := add(a, b)
    let d := add(a, calldataload(7))
    let e := sub(add(a, 0x1000000), b)
    let f := sub(add(a, 7), 1)
}
// ----
// valueConstraintBasedSimplifier
// a:
//     min: 0
//     max: 65535
//    minB: 0
//    maxB: 65535
// b:
//     min: 0
//     max: 65535
//    minB: 0
//    maxB: 65535
// c:
//     min: 0
//     max: 131070
//    minB: 0
//    maxB: 131071
// d:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// e:
//     min: 16711681
//     max: 0x0100ffff
//    minB: 0
//    maxB: 0x02 * 2**24 - 1
// f:
//     min: 6
//     max: 65541
//    minB: 0
//    maxB: 131071
// {
//     let a := and(calldataload(0), 0xffff)
//     let b := and(calldataload(1), 0xffff)
//     let c := add(a, b)
//     let d := add(a, calldataload(7))
//     let e := sub(add(a, 0x1000000), b)
//     let f := sub(add(a, 7), 1)
// }
