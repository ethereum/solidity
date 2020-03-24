{
    { let c:u256 let b:u256 }
    function f(a:u256, c:u256) -> b:u256 { let x:u256 }
    {
        let a:u256 let x:u256
    }
}
// ====
// dialect: yul
// ----
// step: disambiguator
//
// {
//     {
//         let c
//         let b
//     }
//     function f(a, c_1) -> b_2
//     { let x }
//     {
//         let a_3
//         let x_4
//     }
// }
