{
    if sub(2, 1) {
        mstore(0x40, 0x20)
        for {} sub(1, 1) {} {}
    }
    sstore(1, mload(0x40))
}
// ----
// step: fullSuite
//
// {
//     {
//         mstore(0x40, 0x20)
//         for { } iszero(1) { }
//         { }
//         sstore(1, mload(0x40))
//     }
// }
