{
    function is_prime(x) -> res {
        if lt(x, 2)
        { leave }

        if eq(x, 2)
        { res := true leave }

        if iszero(mod(x, 2))
        { leave }

        for { let i := 3 } iszero(gt(mul(i, i), x)) { i := add(i, 2) }
        {
            if iszero(mod(x, i))
            { leave }
        }
        res := true
    }

    function c0() -> r { r := is_prime(0) }
    function c1() -> r { r := is_prime(1) }
    function c2() -> r { r := is_prime(2) }
    function c3() -> r { r := is_prime(3) }
    function c4() -> r { r := is_prime(4) }
    function c5() -> r { r := is_prime(5) }
    function c6() -> r { r := is_prime(6) }
    function c7() -> r { r := is_prime(7) }
    function c9() -> r { r := is_prime(9) }
    function c69() -> r { r := is_prime(69) }
    function c420() -> r { r := is_prime(420) }
    // https://en.wikipedia.org/wiki/List_of_prime_numbers#Bell_primes
    function c27644437() -> r { r := is_prime(27644437) }
    // https://en.wikipedia.org/wiki/List_of_prime_numbers#Circular_primes
    function c999331() -> r { r := is_prime(999331) }

    // The following are too big. Need better algorithm.
    function c1e9plus7() -> r { r := is_prime(add(exp(10, 9), 7)) }
    function c998244353() -> r { r := is_prime(998244353) }
}
// ----
// step: constantFunctionEvaluator
//
// {
//     function is_prime(x) -> res
//     {
//         if lt(x, 2) { leave }
//         if eq(x, 2)
//         {
//             res := true
//             leave
//         }
//         if iszero(mod(x, 2)) { leave }
//         for { let i := 3 } iszero(gt(mul(i, i), x)) { i := add(i, 2) }
//         {
//             if iszero(mod(x, i)) { leave }
//         }
//         res := true
//     }
//     function c0() -> r
//     { r := 0 }
//     function c1() -> r_1
//     { r_1 := 0 }
//     function c2() -> r_2
//     { r_2 := 1 }
//     function c3() -> r_3
//     { r_3 := 1 }
//     function c4() -> r_4
//     { r_4 := 0 }
//     function c5() -> r_5
//     { r_5 := 1 }
//     function c6() -> r_6
//     { r_6 := 0 }
//     function c7() -> r_7
//     { r_7 := 1 }
//     function c9() -> r_8
//     { r_8 := 0 }
//     function c69() -> r_9
//     { r_9 := 0 }
//     function c420() -> r_10
//     { r_10 := 0 }
//     function c27644437() -> r_11
//     { r_11 := 1 }
//     function c999331() -> r_12
//     { r_12 := 1 }
//     function c1e9plus7() -> r_13
//     {
//         r_13 := is_prime(add(exp(10, 9), 7))
//     }
//     function c998244353() -> r_14
//     { r_14 := is_prime(998244353) }
// }
