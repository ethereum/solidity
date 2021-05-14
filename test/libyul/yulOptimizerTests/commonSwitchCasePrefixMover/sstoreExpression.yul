{
    function f() -> x {
        x := calldataload(0)
        sstore(0, 42)
    }
    switch f()
    case 0 {
        let x := sload(0)
        sstore(1, x)
    }
    default {
        let x := sload(0)
        sstore(1, x)
    }
}
// ----
// step: commonSwitchCasePrefixMover
//
// {
//     switch f()
//     case 0 {
//         let x_1 := sload(0)
//         sstore(1, x_1)
//     }
//     default {
//         let x_2 := sload(0)
//         sstore(1, x_2)
//     }
//     function f() -> x
//     {
//         x := calldataload(0)
//         sstore(0, 42)
//     }
// }
