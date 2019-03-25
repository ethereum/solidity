{
    let x := 7
    let a := eq(x, x)
    let b := eq(x, and(calldataload(0), 1)) // not equal
    let c := eq(calldataload(0), calldataload(1))
}
// ----
// valueConstraintBasedSimplifier
// x:
//        = 7
// a:
//        = 1
// b:
//        = 0
// c:
//     min: 0
//     max: 1
//    minB: 0
//    maxB: 1
// {
//     let x := 7
//     let a := 1
//     let b := 0
//     let c := eq(calldataload(0), calldataload(1))
// }
