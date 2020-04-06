{
    let a:u256
    function f() { let b:u256 }
    let c:u256
    function g() { let d:u256 }
    let e:u256
}
// ====
// dialect: yul
// ----
// step: mainFunction
//
// {
//     function main()
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
