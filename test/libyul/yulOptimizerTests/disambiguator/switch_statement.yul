{
    { let a, b, c }
    {
        let a
        switch a
        case 0 { let b := a }
        default { let c := a }
    }
}
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
