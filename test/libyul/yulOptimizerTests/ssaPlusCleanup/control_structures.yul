{
    function copy(from, to) -> length {
        length := mload(from)
        mstore(to, length)
        from := add(from, 0x20)
        to := add(to, 0x20)
        for { let x := 1 } lt(x, length) { x := add(x, 0x20) } {
            mstore(add(to, x), mload(add(from, x)))
        }
    }
}
// ----
// ssaPlusCleanup
// {
//     function copy(from, to) -> length
//     {
//         let length_1 := mload(from)
//         length := length_1
//         mstore(to, length_1)
//         let from_2 := add(from, 0x20)
//         let to_3 := add(to, 0x20)
//         for {
//             let x_4 := 1
//             let x := x_4
//         }
//         lt(x, length_1)
//         {
//             let x_5 := add(x, 0x20)
//             x := x_5
//         }
//         {
//             mstore(add(to_3, x), mload(add(from_2, x)))
//         }
//     }
// }
