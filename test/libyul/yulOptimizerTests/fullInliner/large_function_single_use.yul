{
	function f(a) -> b {
        let x := mload(a)
        b := sload(x)
        let c := 3
        mstore(mul(a, b), mload(x))
        let y := add(a, x)
        sstore(y, 10)
    }
    // Single-use functions are always inlined.
    let r := f(mload(1))
}
// ====
// step: fullInliner
// ----
// {
//     {
//         let a_6 := mload(1)
//         let b_7 := 0
//         let x_8 := mload(a_6)
//         b_7 := sload(x_8)
//         let c_9 := 3
//         mstore(mul(a_6, b_7), mload(x_8))
//         let y_12 := add(a_6, x_8)
//         sstore(y_12, 10)
//         let r := b_7
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
