{
    { let a:u256, b:u256, c:u256 }
    {
        let a:bool
        if a { let b:bool := a }
    }
}
// ====
// step: disambiguator
// yul: true
// ----
// {
//     { let a:u256, b:u256, c:u256 }
//     {
//         let a_1:bool
//         if a_1 { let b_2:bool := a_1 }
//     }
// }
