{
    let x := 1
    let y := 2
    let a := 3
    let b := 4
    for {} 1 {} {
    for {} 1 {} {
    for {} 1 {} {
    // Here, the nesting is not yet too deep, so the assignment
    // should be removed.
    for {} 1 { x := 9 } {
        y := 10
        for {} 1 {} {
        for {} 1 {} {
        // Now we are too deep, assignments stay.
        for {} 1 { a := 10 } {
            b := 12
            b := 11
        }
        }
        }
    }
    }
    }
    }
    x := 13
}
// ====
// step: redundantAssignEliminator
// ----
// {
//     let x := 1
//     let y := 2
//     let a := 3
//     let b := 4
//     for {
//     }
//     1
//     {
//     }
//     {
//         for {
//         }
//         1
//         {
//         }
//         {
//             for {
//             }
//             1
//             {
//             }
//             {
//                 for {
//                 }
//                 1
//                 {
//                 }
//                 {
//                     for {
//                     }
//                     1
//                     {
//                     }
//                     {
//                         for {
//                         }
//                         1
//                         {
//                         }
//                         {
//                             for {
//                             }
//                             1
//                             {
//                                 a := 10
//                             }
//                             {
//                                 b := 12
//                                 b := 11
//                             }
//                         }
//                     }
//                 }
//             }
//         }
//     }
// }
