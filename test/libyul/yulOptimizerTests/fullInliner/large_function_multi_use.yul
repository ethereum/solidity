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
// step: fullInliner
//
// {
//     {
//         let a_1 := mload(2)
//         let a2 := 2
//         let a_2 := a_1
//         let b_1 := 0
//         let x_1 := mload(a_2)
//         b_1 := sload(x_1)
//         let y_1 := add(a_2, x_1)
//         sstore(y_1, 10)
//         let r := b_1
//         let a_3 := a2
//         let b_2 := 0
//         let x_2 := mload(a_3)
//         b_2 := sload(x_2)
//         let y_2 := add(a_3, x_2)
//         sstore(y_2, 10)
//         let t := b_2
//         let a3
//         let a_4 := a3
//         let b_3 := 0
//         let x_3 := mload(a_4)
//         b_3 := sload(x_3)
//         let y_3 := add(a_4, x_3)
//         sstore(y_3, 10)
//         let s := b_3
//     }
//     function f(a) -> b
//     {
//         let x := mload(a)
//         b := sload(x)
//         let y := add(a, x)
//         sstore(y, 10)
//     }
// }
