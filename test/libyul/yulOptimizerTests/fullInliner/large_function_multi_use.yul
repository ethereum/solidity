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
//         let r := f(a_1)
//         let f_a := a2
//         let f_b := 0
//         let f_x := mload(f_a)
//         f_b := sload(f_x)
//         let f_y := add(f_a, f_x)
//         sstore(f_y, 10)
//         let t := f_b
//         let a3
//         let f_a_3 := a3
//         let f_b_4 := 0
//         let f_x_5 := mload(f_a_3)
//         f_b_4 := sload(f_x_5)
//         let f_y_6 := add(f_a_3, f_x_5)
//         sstore(f_y_6, 10)
//         let s := f_b_4
//     }
//     function f(a) -> b
//     {
//         let x := mload(a)
//         b := sload(x)
//         let y := add(a, x)
//         sstore(y, 10)
//     }
// }
