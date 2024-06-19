{
    function copy(from, to) -> length {
        length := mload(from)
        mstore(to, length)
        from := add(from, 0x20)
        to := add(to, 0x20)
        let x := 1
        for {  } lt(x, length) { x := add(x, 0x20) } {
            mstore(add(to, x), mload(add(from, x)))
        }
    }
}
// ----
// step: ssaPlusCleanup
//
// {
//     function copy(from, to) -> length
//     {
//         let from_2 := from
//         let to_2 := to
//         let length_1 := mload(from_2)
//         length := length_1
//         mstore(to_2, length_1)
//         let from_1 := add(from_2, 0x20)
//         let to_1 := add(to_2, 0x20)
//         let x_1 := 1
//         let x := x_1
//         for { }
//         lt(x, length_1)
//         {
//             let x_4 := x
//             let x_2 := add(x_4, 0x20)
//             x := x_2
//         }
//         {
//             let x_3 := x
//             mstore(add(to_1, x_3), mload(add(from_1, x_3)))
//         }
//     }
// }
