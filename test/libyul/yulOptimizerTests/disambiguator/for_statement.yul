{
    { let a:u256, b:u256 }
    {
        for { let a:u256 } a { a := a } {
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
//         for { let a_1 } a_1 { a_1 := a_1 }
//         { let b_2 := a_1 }
//     }
// }
