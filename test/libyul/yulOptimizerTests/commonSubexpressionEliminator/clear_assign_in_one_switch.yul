{
    let x := 10
    switch calldataload(0)
    case 0 {
        x := 2
    }
    case 1 {
        if calldataload(2) { revert(0, 0) }
    }
    // this should not be replaced by x
    sstore(0, 10)

    function f(arg) -> r {
        switch calldataload(0)
        case 0 {
            r := 2
        }
        case 1 {
            if calldataload(2) { revert(0, 0) }
        }
        // this should not be replaced by r
        sstore(0, 0)
    }
}
// ----
// step: commonSubexpressionEliminator
//
// {
//     let x := 10
//     switch calldataload(0)
//     case 0 { x := 2 }
//     case 1 {
//         if calldataload(2) { revert(0, 0) }
//     }
//     sstore(0, 10)
//     function f(arg) -> r
//     {
//         switch calldataload(0)
//         case 0 { r := 2 }
//         case 1 {
//             if calldataload(2) { revert(0, 0) }
//         }
//         sstore(0, 0)
//     }
// }
