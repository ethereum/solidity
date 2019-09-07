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
// ====
// step: ssaPlusCleanup
// ----
// {
//     function copy(from, to) -> length
//     {
//         let from_6 := from
//         let to_7 := to
//         let length_1 := mload(from_6)
//         length := length_1
//         mstore(to_7, length_1)
//         let from_2 := add(from_6, 0x20)
//         let to_3 := add(to_7, 0x20)
//         let x_4 := 1
//         let x := x_4
//         for { }
//         lt(x, length_1)
//         {
//             let x_9 := x
//             let x_5 := add(x_9, 0x20)
//             x := x_5
//         }
//         {
//             let x_8 := x
//             mstore(add(to_3, x_8), mload(add(from_2, x_8)))
//         }
//     }
// }
