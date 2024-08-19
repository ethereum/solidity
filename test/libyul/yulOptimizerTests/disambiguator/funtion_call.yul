{
    { let a, b, c, d, f }
    {
        function f(a) -> c, d {
            let b, c_1 := f(a)
        }
    }
}
// ----
// step: disambiguator
//
// {
//     { let a, b, c, d, f }
//     {
//         function f_1(a_2) -> c_3, d_4
//         { let b_5, c_1 := f_1(a_2) }
//     }
// }
