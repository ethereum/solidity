{
    let a := 1
    switch calldataload(0)
    case 0 {
        mstore(0, 1)
        a := 8
    }
    default {
        a := 3
        a := 4
    }
    a := 5
}
// ====
// step: blockDeepener
// ----
// {
//     let a := 1
//     {
//         switch calldataload(0)
//         case 0 {
//             mstore(0, 1)
//             { a := 8 }
//         }
//         default {
//             a := 3
//             { a := 4 }
//         }
//         { a := 5 }
//     }
// }
