{
    let a:u256
    function f() {
        let b:u256
        function g() {
            let c:u256
        }
        let d:u256
    }
}
// ====
// dialect: yul
// ----
// step: functionGrouper
//
// {
//     { let a }
//     function f()
//     {
//         let b
//         function g()
//         { let c }
//         let d
//     }
// }
