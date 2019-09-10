{
	function f(a) -> b {
        let x := mload(a)
        b := sload(x)
    }
    // This will stop inlining at some point because
    // the function gets too big.
    function g() -> x {
        x := f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(f(2)))))))))))))))))))
    }
}
// ====
// step: fullInliner
// ----
// {
//     function f(a) -> b
//     { b := sload(mload(a)) }
//     function g() -> x_1
//     {
//         let a_20 := 2
//         let b_21 := 0
//         b_21 := sload(mload(a_20))
//         let a_23 := b_21
//         let b_24 := 0
//         b_24 := sload(mload(a_23))
//         let a_26 := b_24
//         let b_27 := 0
//         b_27 := sload(mload(a_26))
//         let a_29 := b_27
//         let b_30 := 0
//         b_30 := sload(mload(a_29))
//         let a_32 := b_30
//         let b_33 := 0
//         b_33 := sload(mload(a_32))
//         let a_35 := b_33
//         let b_36 := 0
//         b_36 := sload(mload(a_35))
//         let a_38 := b_36
//         let b_39 := 0
//         b_39 := sload(mload(a_38))
//         let a_41 := b_39
//         let b_42 := 0
//         b_42 := sload(mload(a_41))
//         let a_44 := b_42
//         let b_45 := 0
//         b_45 := sload(mload(a_44))
//         let a_47 := b_45
//         let b_48 := 0
//         b_48 := sload(mload(a_47))
//         let a_50 := b_48
//         let b_51 := 0
//         b_51 := sload(mload(a_50))
//         let a_53 := b_51
//         let b_54 := 0
//         b_54 := sload(mload(a_53))
//         let a_56 := b_54
//         let b_57 := 0
//         b_57 := sload(mload(a_56))
//         x_1 := f(f(f(f(f(f(b_57))))))
//     }
// }
