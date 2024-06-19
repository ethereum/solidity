{
    { let a:u256, b:u256, c:u256, d:u256, f:u256 }
    {
        function f(a:u256) -> c:u256, d:u256 {
            let b:u256, c_1:u256 := f(a)
        }
    }
}
// ====
// dialect: yul
// ----
// step: disambiguator
//
// {
//     { let a, b, c, d, f }
//     {
//         function f_1(a_1) -> c_1, d_1
//         { let b_1, c_1_1 := f_1(a_1) }
//     }
// }
