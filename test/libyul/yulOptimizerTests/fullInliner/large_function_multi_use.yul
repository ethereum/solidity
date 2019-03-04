{
	function f(a) -> b {
        let x := mload(a)
        b := sload(x)
        let y := add(a, x)
        sstore(y, 10)
    }
    let a := mload(2)
    let a2 := 2
    // This should not be inlined because it is not a constant
    let r := f(a)
    // This should be inlined because it is a constant
    let t := f(a2)
    let a3
    // This should be inlined because it is a constant as well (zero)
    let s := f(a3)
}
// ----
// fullInliner
// {
//     {
//         let a_1 := mload(2)
//         let a2 := 2
//         let a_3 := a_1
//         let b_4 := 0
//         let x_5 := mload(a_3)
//         b_4 := sload(x_5)
//         let y_6 := add(a_3, x_5)
//         sstore(y_6, 10)
//         let r := b_4
//         let a_8 := a2
//         let b_9 := 0
//         let x_10 := mload(a_8)
//         b_9 := sload(x_10)
//         let y_11 := add(a_8, x_10)
//         sstore(y_11, 10)
//         let t := b_9
//         let a3
//         let a_13 := a3
//         let b_14 := 0
//         let x_15 := mload(a_13)
//         b_14 := sload(x_15)
//         let y_16 := add(a_13, x_15)
//         sstore(y_16, 10)
//         let s := b_14
//     }
//     function f(a) -> b
//     {
//         let x := mload(a)
//         b := sload(x)
//         let y := add(a, x)
//         sstore(y, 10)
//     }
// }
