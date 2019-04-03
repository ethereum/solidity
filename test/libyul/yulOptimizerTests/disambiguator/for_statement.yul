{
    { let a:u256, b:u256 }
    {
        for { let a:u256 } a { a := a } {
            let b:u256 := a
        }
    }
}
// ====
// step: disambiguator
// yul: true
// ----
// {
//     {
//         let a:u256, b:u256
//     }
//     {
//         for {
//             let a_1:u256
//         }
//         a_1
//         {
//             a_1 := a_1
//         }
//         {
//             let b_2:u256 := a_1
//         }
//     }
// }
