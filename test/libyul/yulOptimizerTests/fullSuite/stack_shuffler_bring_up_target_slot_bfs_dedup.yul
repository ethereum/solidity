// This is a regression test in context of https://github.com/argotorg/solidity/pull/16499:
// A BFS deduplication performance bug in `bringUpTargetSlot` where the same offset could be enqueued multiple times
// before being visited, by marking offsets as seen on enqueue rather than on dequeue.
{
    mstore(memoryguard(0x10000), 1)
    sstore(mload(calldataload(0)), 1)
    {
        function foo_n_0(x_4, x_5)
        {
            if x_5 {
                x_4 := 0x2000000000001
            }
            x_4 := call(
                call(
                    blockhash(0x200000000001),
                    0x20000000001,
                    0x2000000001,
                    mod("h", 32768),
                    mod(0x200000001, 32768),
                    mod(0x20000001, 32768),
                    mod(0x2000001, 32768)
                ),
                0x200001,
                0x20001,
                mod(0x2001, 32768),
                mod(calldatasize(), 32768),
                mod(calldatasize(), 32768),
                mod(
                    call(
                        call(
                            x_4,
                            0x201,
                            number(),
                            mod(0x21, 32768),
                            mod(0x3, 32768),
                            mod(0x3f, 32768),
                            mod(address(), 32768)
                        ),
                        0x3ff,
                        0x3fff,
                        mod(0x3ffff, 32768),
                        mod(9042383626829830, 32768),
                        mod(0x3fffff, 32768),
                        mod(0x3ffffff, 32768)
                    ),
                    32768
                )
            )
        }
        foo_n_0(0x200000000000000001, 0x20000000000000001)
        foo_n_0(sload(224),calldataload(288))
        foo_n_0(0x3ffffffffffff, 0x3fffffffffffff)
        foo_n_0(0x3fffffffffffffff, 0x3ffffffffffffffff)
        foo_n_0(0x3ffffffffffffffffff, 0x3fffffffffffffffffff)
    }
}
// ----
// step: fullSuite
//
// {
//     {
//         mstore(memoryguard(0x010000), 1)
//         sstore(mload(calldataload(0)), 1)
//         let x := 0x200000000000000001
//         x := 0x2000000000001
//         let _1 := and(call(call(x, 0x201, number(), 0x21, 0x3, 0x3f, and(address(), 32767)), 0x3ff, 0x3fff, 32767, 6, 32767, 32767), 32767)
//         pop(call(call(blockhash(0x200000000001), 0x20000000001, 0x2000000001, and("h", 32767), 1, 1, 1), 0x200001, 0x20001, 0x2001, and(calldatasize(), 32767), and(calldatasize(), 32767), _1))
//         let x_1 := sload(224)
//         if calldataload(288) { x_1 := x }
//         pop(call(call(blockhash(0x200000000001), 0x20000000001, 0x2000000001, and("h", 32767), 1, 1, 1), 0x200001, 0x20001, 0x2001, and(calldatasize(), 32767), and(calldatasize(), 32767), and(call(call(x_1, 0x201, number(), 0x21, 0x3, 0x3f, and(address(), 32767)), 0x3ff, 0x3fff, 32767, 6, 32767, 32767), 32767)))
//         let x_2 := 0x3ffffffffffff
//         x_2 := x
//         pop(call(call(blockhash(0x200000000001), 0x20000000001, 0x2000000001, and("h", 32767), 1, 1, 1), 0x200001, 0x20001, 0x2001, and(calldatasize(), 32767), and(calldatasize(), 32767), and(call(call(x, 0x201, number(), 0x21, 0x3, 0x3f, and(address(), 32767)), 0x3ff, 0x3fff, 32767, 6, 32767, 32767), 32767)))
//         let x_3 := 0x3fffffffffffffff
//         x_3 := x
//         pop(call(call(blockhash(0x200000000001), 0x20000000001, 0x2000000001, and("h", 32767), 1, 1, 1), 0x200001, 0x20001, 0x2001, and(calldatasize(), 32767), and(calldatasize(), 32767), and(call(call(x, 0x201, number(), 0x21, 0x3, 0x3f, and(address(), 32767)), 0x3ff, 0x3fff, 32767, 6, 32767, 32767), 32767)))
//         let x_4 := 0x3ffffffffffffffffff
//         x_4 := x
//         pop(call(call(blockhash(0x200000000001), 0x20000000001, 0x2000000001, and("h", 32767), 1, 1, 1), 0x200001, 0x20001, 0x2001, and(calldatasize(), 32767), and(calldatasize(), 32767), and(call(call(x, 0x201, number(), 0x21, 0x3, 0x3f, and(address(), 32767)), 0x3ff, 0x3fff, 32767, 6, 32767, 32767), 32767)))
//     }
// }
