{
    sstore(f(1), 1)
    sstore(f(2), 1)
    sstore(f(3), 1)
    function f(a) -> x {
        for {let b := 10} iszero(b) { b := sub(b, 1) }
        {
            a := calldataload(0)
            mstore(a, x)
            // to prevent f from getting inlined
            if iszero(a) { leave }
        }
    }
}
// ====
// EVMVersion: >=shanghai
// ----
// step: fullSuite
//
// {
//     {
//         f_63()
//         f()
//         f_65()
//         sstore(0, 1)
//     }
//     function f_63()
//     {
//         let b := 10
//         let a := calldataload(0)
//         let _1 := iszero(a)
//         for { } iszero(b) { b := add(b, not(0)) }
//         {
//             mstore(a, 0)
//             if _1 { leave }
//         }
//     }
//     function f()
//     {
//         let b := 10
//         let a := calldataload(0)
//         let _1 := iszero(a)
//         for { } iszero(b) { b := add(b, not(0)) }
//         {
//             mstore(a, 0)
//             if _1 { leave }
//         }
//     }
//     function f_65()
//     {
//         let b := 10
//         let a := calldataload(0)
//         let _1 := iszero(a)
//         for { } iszero(b) { b := add(b, not(0)) }
//         {
//             mstore(a, 0)
//             if _1 { leave }
//         }
//     }
// }
