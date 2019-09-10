{
    { let c:u256 let b:u256 }
    function f(a:u256, c:u256) -> b:u256 { let x:u256 }
    {
        let a:u256 let x:u256
    }
}
// ====
// step: disambiguator
// yul: true
// ----
// {
//     {
//         let c:u256
//         let b:u256
//     }
//     function f(a:u256, c_1:u256) -> b_2:u256
//     { let x:u256 }
//     {
//         let a_3:u256
//         let x_4:u256
//     }
// }
