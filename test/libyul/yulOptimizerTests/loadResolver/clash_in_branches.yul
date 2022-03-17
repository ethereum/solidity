{
    let x := calldataload(0)
    let y := calldataload(1)
    sstore(x, y)
    switch calldataload(2)
    case 0 {
        x := 20
        sstore(x, y)
    }
    default {
        x := 30
        sstore(x, y)
    }
    let t := sload(x)
    sstore(0, t)
}
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 0
//         let x := calldataload(_1)
//         let y := calldataload(1)
//         sstore(x, y)
//         switch calldataload(2)
//         case 0 {
//             x := 20
//             sstore(x, y)
//         }
//         default {
//             x := 30
//             sstore(x, y)
//         }
//         sstore(_1, sload(x))
//     }
// }
