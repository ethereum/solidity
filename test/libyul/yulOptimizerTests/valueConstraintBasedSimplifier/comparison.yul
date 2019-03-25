{
    let a := and(calldataload(0), 0xf)
    let b := and(calldataload(1), 0xf)
    let c := add(b, 2) // overlaps with a
    let d := add(b, 0x100) // does not overlap with a
    let e := lt(a, c)
    let f := lt(a, d)
    let g := lt(c, a)
    let h := lt(d, a)
    let r := lt(a, a) // currently unknown, might change later.
    let x := gt(d, a)
    let y := gt(c, a)
}
// ----
// valueConstraintBasedSimplifier
// a:
//     min: 0
//     max: 15
//    minB: 0
//    maxB: 15
// b:
//     min: 0
//     max: 15
//    minB: 0
//    maxB: 15
// c:
//     min: 2
//     max: 17
//    minB: 0
//    maxB: 31
// d:
//     min: 256
//     max: 271
//    minB: 256
//    maxB: 271
// e:
//     min: 0
//     max: 1
//    minB: 0
//    maxB: 1
// f:
//        = 1
// g:
//     min: 0
//     max: 1
//    minB: 0
//    maxB: 1
// h:
//        = 0
// r:
//     min: 0
//     max: 1
//    minB: 0
//    maxB: 1
// x:
//        = 1
// y:
//     min: 0
//     max: 1
//    minB: 0
//    maxB: 1
// {
//     let a := and(calldataload(0), 0xf)
//     let b := and(calldataload(1), 0xf)
//     let c := add(b, 2)
//     let d := add(b, 0x100)
//     let e := lt(a, c)
//     let f := 1
//     let g := lt(c, a)
//     let h := 0
//     let r := lt(a, a)
//     let x := 1
//     let y := gt(c, a)
// }
