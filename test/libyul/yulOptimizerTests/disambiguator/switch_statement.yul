// yul
{
    { let a:u256, b:u256, c:u256 }
    {
        let a:u256
        switch a
        case 0:u256 { let b:u256 := a }
        default { let c:u256 := a }
    }
}
// ----
// disambiguator
// {
//     {
//         let a:u256, b:u256, c:u256
//     }
//     {
//         let a_1:u256
//         switch a_1
//         case 0:u256 {
//             let b_1:u256 := a_1
//         }
//         default {
//             let c_1:u256 := a_1
//         }
//     }
// }
