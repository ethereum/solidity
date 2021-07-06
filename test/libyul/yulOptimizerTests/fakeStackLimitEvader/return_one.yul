{
    {
        mstore(0x40, memoryguard(0x80))
        sstore(0, f())
    }
    function f() -> $b1 {
        if calldataload(0) {
            $b1 := 0
            leave
        }
        $b1 := 1
    }

}
// ----
// step: fakeStackLimitEvader
//
// {
//     {
//         mstore(0x40, memoryguard(0xa0))
//         sstore(0, f())
//     }
//     function f() -> $b1
//     {
//         mstore(0x80, 0)
//         f_1()
//         $b1 := mload(0x80)
//     }
//     function f_1()
//     {
//         if calldataload(0)
//         {
//             mstore(0x80, 0)
//             leave
//         }
//         mstore(0x80, 1)
//     }
// }
