{
    for {} 1 {} {
    for {} 1 {} {
    for {} 1 {} {
    for {} 1 {} {
    for {} 1 {} {
    for {} 1 {} {
    for {} 1 { let a := 1 a := 2 a := 3 } {
        // Declarations inside body and post should be handled as normal.
        let b := 1
        b := 2
        b := 3
    }
    }
    }
    }
    }
    }
    }
}
// ====
// step: redundantAssignEliminator
// ----
// {
//     for { } 1 { }
//     {
//         for { } 1 { }
//         {
//             for { } 1 { }
//             {
//                 for { } 1 { }
//                 {
//                     for { } 1 { }
//                     {
//                         for { } 1 { }
//                         {
//                             for { } 1 { let a := 1 }
//                             { let b := 1 }
//                         }
//                     }
//                 }
//             }
//         }
//     }
// }
