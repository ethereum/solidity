{
    let a
    function f() {
        let b
        function g() { let c }
        let d
    }
}
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
