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
//         let from_1 := add(from, 0x20)
//         let to_1 := add(to, 0x20)
//         for {
//             let x_1 := 1
//             let x := x_1
//         }
//         lt(x, length_1)
//         {
//             let x_2 := add(x, 0x20)
//             x := x_2
//         }
//         {
//             mstore(add(to_1, x), mload(add(from_1, x)))
//         }
//     }
// }
