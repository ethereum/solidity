{
	function f(a) -> b {
        let x := mload(a)
        b := sload(x)
        let c := 3
        mstore(mul(a, b), mload(x))
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
//         let f_b
//         let f_x := mload(f_a)
//         f_b := sload(f_x)
//         let f_c := 3
//         mstore(mul(f_a, f_b), mload(f_x))
//         let f_y := add(f_a, f_x)
//         sstore(f_y, 10)
//         let t := f_b
//         let a3
//         let f_a_5 := a3
//         let f_b_6
//         let f_x_7 := mload(f_a_5)
//         f_b_6 := sload(f_x_7)
//         let f_c_8 := 3
//         mstore(mul(f_a_5, f_b_6), mload(f_x_7))
//         let f_y_11 := add(f_a_5, f_x_7)
//         sstore(f_y_11, 10)
//         let s := f_b_6
//     }
//     function f(a) -> b
//     {
//         let x := mload(a)
//         b := sload(x)
//         let c := 3
//         mstore(mul(a, b), mload(x))
//         let y := add(a, x)
//         sstore(y, 10)
//     }
// }
