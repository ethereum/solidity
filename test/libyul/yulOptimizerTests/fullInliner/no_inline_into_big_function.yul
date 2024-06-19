{
    function f(a) -> b {
        let x := mload(a)
        b := sload(x)
    }
    // This will stop inlining at some point because
    // the function gets too big.
    function g() -> x {
        x := f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(2)))))))))))))))))))
    }
}
// ====
// EVMVersion: >homestead
// ----
// step: fullInliner
//
// {
//     { }
//     function f(a) -> b
//     { b := sload(mload(a)) }
//     function g() -> x
//     {
//         let a_1 := 2
//         let b_1 := 0
//         b_1 := sload(mload(a_1))
//         let a_2 := b_1
//         let b_2 := 0
//         b_2 := sload(mload(a_2))
//         let a_3 := b_2
//         let b_3 := 0
//         b_3 := sload(mload(a_3))
//         let a_4 := b_3
//         let b_4 := 0
//         b_4 := sload(mload(a_4))
//         let a_5 := b_4
//         let b_5 := 0
//         b_5 := sload(mload(a_5))
//         let a_6 := b_5
//         let b_6 := 0
//         b_6 := sload(mload(a_6))
//         let a_7 := b_6
//         let b_7 := 0
//         b_7 := sload(mload(a_7))
//         let a_8 := b_7
//         let b_8 := 0
//         b_8 := sload(mload(a_8))
//         let a_9 := b_8
//         let b_9 := 0
//         b_9 := sload(mload(a_9))
//         let a_10 := b_9
//         let b_10 := 0
//         b_10 := sload(mload(a_10))
//         let a_11 := b_10
//         let b_11 := 0
//         b_11 := sload(mload(a_11))
//         let a_12 := b_11
//         let b_12 := 0
//         b_12 := sload(mload(a_12))
//         let a_13 := b_12
//         let b_13 := 0
//         b_13 := sload(mload(a_13))
//         x := f(f(f(f(f(f(b_13))))))
//     }
// }
