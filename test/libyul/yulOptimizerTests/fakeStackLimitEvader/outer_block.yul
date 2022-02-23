{
    mstore(0x40, memoryguard(0x80))
    let $x := 42
    sstore(42, $x)
}
// ----
// step: fakeStackLimitEvader
//
// {
//     mstore(0x40, memoryguard(0xa0))
//     mstore(0x80, 42)
//     sstore(42, mload(0x80))
// }
