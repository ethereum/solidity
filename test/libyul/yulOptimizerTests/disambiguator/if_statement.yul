// yul
{
    { let a:u256, b:u256, c:u256 }
    {
        let a:bool
        if a { let b:bool := a }
    }
}
// ----
// disambiguator
// {
//     {
//         let a:u256, b:u256, c:u256
//     }
//     {
//         let a_1:bool
//         if a_1
//         {
//             let b_1:bool := a_1
//         }
//     }
// }
