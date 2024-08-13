{
    let a
    function f() { let b }
    let c function g() { let d }
    let e
}
// ----
// step: functionGrouper
//
// {
//     {
//         let a
//         let c
//         let e
//     }
//     function f()
//     { let b }
//     function g()
//     { let d }
// }
