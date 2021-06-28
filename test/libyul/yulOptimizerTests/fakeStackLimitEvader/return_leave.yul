{
    {
        mstore(0x40, memoryguard(0x80))
        let a1, a2 := f()
        sstore(a1, a2)
    }
    function g(x) -> a, b { a := x b := 2 }
    function f() -> $b1, $b2 {
        if calldataload(0) {
            $b1, $b2 := g(1)
            leave
        }
        $b1, $b2 := g(2)
    }

}
// ----
// step: fakeStackLimitEvader
//
// {
//     {
//         mstore(0x40, memoryguard(0xc0))
//         f()
//         let a2 := mload(0x80)
//         let a1 := mload(0xa0)
//         sstore(a1, a2)
//     }
//     function g(x) -> a, b
//     {
//         a := x
//         b := 2
//     }
//     function f()
//     {
//         mstore(0xa0, 0)
//         mstore(0x80, 0)
//         if calldataload(0)
//         {
//             let $b1_1, $b2_2 := g(1)
//             mstore(0x80, $b2_2)
//             mstore(0xa0, $b1_1)
//             leave
//         }
//         let $b1_3, $b2_4 := g(2)
//         mstore(0x80, $b2_4)
//         mstore(0xa0, $b1_3)
//     }
// }
