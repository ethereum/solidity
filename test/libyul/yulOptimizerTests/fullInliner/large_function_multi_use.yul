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
