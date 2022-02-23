{
    function f(x) -> y { y := x }
    mstore(0x40, memoryguard(0))

    let $z := 42
    $z := f($z)
}
// ----
// step: fakeStackLimitEvader
//
// {
//     function f(x) -> y
//     { y := x }
//     mstore(0x40, memoryguard(0x20))
//     mstore(0x00, 42)
//     mstore(0x00, f(mload(0x00)))
// }
