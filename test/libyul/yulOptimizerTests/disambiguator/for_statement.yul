{
    { let a:u256, b:u256 }
    {
        function eq(x: u256, y: u256) -> z: bool {}
        for { let a:u256 } eq(a, a) { a := a } {
            let b:u256 := a
        }
    }
}
// ====
// dialect: yul
// step: disambiguator
// ----
// {
//     { let a, b }
//     {
//         function eq(x, y) -> z:bool
//         { }
//         for { let a_1 } eq(a_1, a_1) { a_1 := a_1 }
//         { let b_2 := a_1 }
//     }
// }
