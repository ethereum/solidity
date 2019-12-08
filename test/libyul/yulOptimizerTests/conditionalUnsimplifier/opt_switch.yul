{
    let x := calldataload(0)
    switch x
    case 0 { x := 0 }
    case 1 { x := 1 }
    case 2 { x := 8 /* wrong literal */ }
    default { }

    pop(x)
}
// ====
// step: conditionalUnsimplifier
// ----
// {
//     let x := calldataload(0)
//     switch x
//     case 0 { }
//     case 1 { }
//     case 2 { x := 8 }
//     default { }
//     pop(x)
// }
