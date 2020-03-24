{
    { let a:u256, b:u256, c:u256 }
    {
        let a:u256
        switch a
        case 0:u256 { let b:u256 := a }
        default { let c:u256 := a }
    }
}
// ====
// dialect: yul
// ----
// step: disambiguator
//
// {
//     { let a, b, c }
//     {
//         let a_1
//         switch a_1
//         case 0 { let b_2 := a_1 }
//         default { let c_3 := a_1 }
//     }
// }
