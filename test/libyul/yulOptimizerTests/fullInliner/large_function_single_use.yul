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
// ----
// step: fullInliner
//
// {
//     {
//         let a_1 := mload(1)
//         let b_1 := 0
//         let x_1 := mload(a_1)
//         b_1 := sload(x_1)
//         let c_1 := 3
//         mstore(mul(a_1, b_1), mload(x_1))
//         let y_1 := add(a_1, x_1)
//         sstore(y_1, 10)
//         let r := b_1
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
