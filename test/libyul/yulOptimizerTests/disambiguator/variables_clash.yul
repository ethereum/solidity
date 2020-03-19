{ { let a:u256 let a_1:u256 } { let a:u256 } }
// ====
// dialect: yul
// ----
// step: disambiguator
//
// {
//     {
//         let a
//         let a_1
//     }
//     { let a_2 }
// }
