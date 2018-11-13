// yul
{
    { let a:u256, b:u256, c:u256, d:u256, f:u256 }
    {
        function f(a:u256) -> c:u256, d:u256 {
            let b:u256, c_1:u256 := f(a)
        }
    }
}
// ----
// disambiguator
// {
//     {
//         let a:u256, b:u256, c:u256, d:u256, f:u256
//     }
//     {
//         function f_1(a_2:u256) -> c_3:u256, d_4:u256
//         {
//             let b_5:u256, c_1:u256 := f_1(a_2)
//         }
//     }
// }
