{
    { let a:u256, b:u256 }
    {
        function eq_function(x: u256, y: u256) -> z: bool {}
        for { let a:u256 } eq_function(a, a) { a := a } {
            let b:u256 := a
        }
    }
}
// ====
// dialect: evmTyped
// ----
// step: disambiguator
//
// {
//     { let a, b }
//     {
//         function eq_function(x, y) -> z:bool
//         { }
//         for { let a_1 } eq_function(a_1, a_1) { a_1 := a_1 }
//         { let b_2 := a_1 }
//     }
// }
