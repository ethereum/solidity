{
    if mul(add(calldataload(0), 2), 3) {
        for { let a := 2 } lt(a, mload(a)) { a := add(a, mul(a, 2)) } {
            let b := mul(add(a, 2), 4)
            sstore(b, mul(b, 2))
        }
    }
}
// ----
// splitJoin
// {
//     if mul(add(calldataload(0), 2), 3)
//     {
//         for {
//             let a := 2
//         }
//         lt(a, mload(a))
//         {
//             a := add(a, mul(a, 2))
//         }
//         {
//             let b := mul(add(a, 2), 4)
//             sstore(b, mul(b, 2))
//         }
//     }
// }
