{
    function f() -> x, y {}
    let a, b := f()
    let d := a
    let e := b
    a := 1
    b := 3
    a, b := f()
    let s := a
    let t := b
}
// ----
// valueConstraintBasedSimplifier
// x:
//        = 0
// y:
//        = 0
// d:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// e:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// a:
//        = 1
// b:
//        = 3
// s:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// t:
//     min: 0
//     max: 2**256 - 1
//    minB: 0
//    maxB: 2**256 - 1
// {
//     function f() -> x, y
//     {
//     }
//     let a, b := f()
//     let d := a
//     let e := b
//     a := 1
//     b := 3
//     a, b := f()
//     let s := a
//     let t := b
// }
