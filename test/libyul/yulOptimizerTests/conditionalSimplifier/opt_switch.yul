{
    let x := calldataload(0)
    switch x
    case 0 { }
    case 1 { }
    default { }

    pop(x)
}
// ====
// step: conditionalSimplifier
// ----
// {
//     let x := calldataload(0)
//     switch x
//     case 0 { x := 0 }
//     case 1 { x := 1 }
//     default { }
//     pop(x)
// }
