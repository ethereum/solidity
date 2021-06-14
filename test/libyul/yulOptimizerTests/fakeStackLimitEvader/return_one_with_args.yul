{
    {
        mstore(0x40, memoryguard(0x80))
        sstore(0, f(1,2,3))
    }
    function f(a, b, c) -> $b1 {
        if calldataload(add(sub(a,b),c)) {
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
//         sstore(0, f(1, 2, 3))
//     }
//     function f(a_2, b_3, c_4) -> $b1
//     {
//         mstore(0x80, 0)
//         f_1(a_2, b_3, c_4)
//         $b1 := mload(0x80)
//     }
//     function f_1(a, b, c)
//     {
//         if calldataload(add(sub(a, b), c))
//         {
//             mstore(0x80, 0)
//             leave
//         }
//         mstore(0x80, 1)
//     }
// }
