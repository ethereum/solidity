{
    { let a, b }
    {
        function eq_function(x, y) -> z {}
        for { let a } eq_function(a, a) { a := a } {
            let b := a
        }
    }
}
// ----
// step: disambiguator
//
// {
//     { let a, b }
//     {
//         function eq_function(x, y) -> z
//         { }
//         for { let a_1 } eq_function(a_1, a_1) { a_1 := a_1 }
//         { let b_2 := a_1 }
//     }
// }
