{
    let a
    function f() {
        let b
        function g() {
            let c
        }
        let d
    }
}
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
