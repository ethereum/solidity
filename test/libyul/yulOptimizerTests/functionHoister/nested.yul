{
    let a:u256
    function f() {
        let b:u256
        function g() { let c:u256 }
        let d:u256
    }
}
// ====
// dialect: evmTyped
// ----
// step: functionHoister
//
// {
//     let a
//     function g()
//     { let c }
//     function f()
//     {
//         let b
//         let d
//     }
// }
