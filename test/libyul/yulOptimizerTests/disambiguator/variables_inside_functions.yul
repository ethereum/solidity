// yul
{
    { let c:u256 let b:u256 }
    function f(a:u256, c:u256) -> b:u256 { let x:u256 }
    {
        let a:u256 let x:u256
    }
}
// ----
// disambiguator
// {
//     {
//         let c:u256
//         let b:u256
//     }
//     function f(a:u256, c_1:u256) -> b_1:u256
//     {
//         let x:u256
//     }
//     {
//         let a_1:u256
//         let x_1:u256
//     }
// }
