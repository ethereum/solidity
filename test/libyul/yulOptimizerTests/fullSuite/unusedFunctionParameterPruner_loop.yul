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
//         f()
//         f_72()
//         f_73()
//         sstore(0, 1)
//     }
//     function f()
//     {
//         let a := 1
//         let b := 10
//         let a_1 := calldataload(0)
//         let _1 := iszero(a_1)
//         for { } iszero(b) { b := add(b, not(0)) }
//         {
//             a := a_1
//             mstore(a_1, 0)
//             if _1 { leave }
//         }
//     }
//     function f_72()
//     {
//         let a := 2
//         let b := 10
//         let a_1 := calldataload(0)
//         let _1 := iszero(a_1)
//         for { } iszero(b) { b := add(b, not(0)) }
//         {
//             a := a_1
//             mstore(a_1, 0)
//             if _1 { leave }
//         }
//     }
//     function f_73()
//     {
//         let a := 3
//         let b := 10
//         let a_1 := calldataload(0)
//         let _1 := iszero(a_1)
//         for { } iszero(b) { b := add(b, not(0)) }
//         {
//             a := a_1
//             mstore(a_1, 0)
//             if _1 { leave }
//         }
//     }
// }
