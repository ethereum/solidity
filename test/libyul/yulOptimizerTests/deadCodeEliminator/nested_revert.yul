{
    let y := mload(0)
    switch y
    case 0 {
        y := 8 }
    case 1 {
        y := 9
        revert(0, 0)
        y := 10
    }
    default {
        y := 10 }
}

// ====
// step: deadCodeEliminator
// ----
// {
//     let y := mload(0)
//     switch y
//     case 0 {
//         y := 8
//     }
//     case 1 {
//         y := 9
//         revert(0, 0)
//     }
//     default {
//         y := 10
//     }
// }
