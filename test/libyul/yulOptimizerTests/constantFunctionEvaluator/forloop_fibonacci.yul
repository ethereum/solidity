{
    function fibonacci(n) -> res {
        let a := 0
        let b := 1
        for {} gt(n, 0) { n := sub(n, 1) }
        {
            let t := b
            b := add(a, b)
            a := t
        }
        res := a
    }

    function f0() -> r
    { r := fibonacci(0) }
    function f1() -> r
    { r := fibonacci(1) }
    function f2() -> r
    { r := fibonacci(2) }
    function f3() -> r
    { r := fibonacci(3) }
    function f4() -> r
    { r := fibonacci(4) }
    function f5() -> r
    { r := fibonacci(5) }
    function f10() -> r
    { r := fibonacci(10) }
    function f11() -> r
    { r := fibonacci(11) }
    function f12() -> r
    { r := fibonacci(12) }
    function f13() -> r
    { r := fibonacci(13) }
    function f20() -> r
    { r := fibonacci(20) }
    function f69() -> r
    { r := fibonacci(69) }
}
// ----
// step: constantFunctionEvaluator
//
// {
//     function fibonacci(n) -> res
//     {
//         let a := 0
//         let b := 1
//         for { } gt(n, 0) { n := sub(n, 1) }
//         {
//             let t := b
//             b := add(a, b)
//             a := t
//         }
//         res := a
//     }
//     function f0() -> r
//     { r := 0 }
//     function f1() -> r_1
//     { r_1 := 1 }
//     function f2() -> r_2
//     { r_2 := 1 }
//     function f3() -> r_3
//     { r_3 := 2 }
//     function f4() -> r_4
//     { r_4 := 3 }
//     function f5() -> r_5
//     { r_5 := 5 }
//     function f10() -> r_6
//     { r_6 := 55 }
//     function f11() -> r_7
//     { r_7 := 89 }
//     function f12() -> r_8
//     { r_8 := 144 }
//     function f13() -> r_9
//     { r_9 := 233 }
//     function f20() -> r_10
//     { r_10 := 6765 }
//     function f69() -> r_11
//     { r_11 := 117669030460994 }
// }
