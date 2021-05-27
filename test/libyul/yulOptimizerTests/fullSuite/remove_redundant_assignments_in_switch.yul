 {
    let x := 0
    switch mload(x)
    case 0 { x := x }
    case 1 { x := 1 }
    default { invalid() }
    mstore(1, 1)
}
// ----
// step: fullSuite
//
// {
//     {
//         switch mload(0)
//         case 0 { }
//         case 1 { }
//         default { invalid() }
//     }
// }
