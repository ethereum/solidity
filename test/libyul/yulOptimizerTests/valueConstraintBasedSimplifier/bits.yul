{
    let a := 0xffff
    let b := or(shl(20, a), and(calldataload(0), 0xff))
    let c := shl(add(300, lt(a, b)), calldataload(0))
    let d := shr(add(300, lt(a, b)), calldataload(0))
    let e := and(b, 0xff)
    let f := not(b)
    let g := byte(calldataload(0), 2)
}
// ----
// valueConstraintBasedSimplifier
// a:
//        = 65535
// b:
//     min: 0x0fFFF00000
//     max: 0x0fFFF000ff
//    minB: 0x0fFFF00000
//    maxB: 0x0fFFF000ff
// c:
//        = 0
// d:
//        = 0
// e:
//     min: 0
//     max: 255
//    minB: 0
//    maxB: 255
// f:
//     min: 0xFFFFffffFFFFffffFFFFffffFFFFffffFFFFffffFFFFffffFFFFfff0000Fff00
//     max: 0xFFFFffffFFFFffffFFFFffffFFFFffffFFFFffffFFFFffffFFFFfff0000Fffff
//    minB: 0xFFFFffffFFFFffffFFFFffffFFFFffffFFFFffffFFFFffffFFFFfff0000Fff00
//    maxB: 0xFFFFffffFFFFffffFFFFffffFFFFffffFFFFffffFFFFffffFFFFfff0000Fffff
// g:
//     min: 0
//     max: 255
//    minB: 0
//    maxB: 255
// {
//     let a := 0xffff
//     let b := or(0x0ffff00000, and(calldataload(0), 0xff))
//     let c := 0
//     let d := 0
//     let e := and(b, 0xff)
//     let f := not(b)
//     let g := byte(calldataload(0), 2)
// }
