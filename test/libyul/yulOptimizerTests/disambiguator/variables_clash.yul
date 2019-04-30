{ { let a:u256 let a_1:u256 } { let a:u256 } }
// ====
// step: disambiguator
// yul: true
// ----
// {
//     {
//         let a:u256
//         let a_1:u256
//     }
//     {
//         let a_2:u256
//     }
// }
