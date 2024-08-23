{
    // should contracted to only function a
    {
        function a() -> r {
            r := add(b(), 1)

            function b() -> x {
                let p, q := c()
                x := add(p, q)

                function c() -> m, n {
                    m := mul(d(), e())
                    n := div(d(), e())

                    function d() -> s {
                        for { let i := 0 } lt(i, 100) { i := add(i, 1) }
                        { s := add(s, i) }
                    }

                    function e() -> s {
                        for { let i := 0 } lt(i, 100) { i := add(i, 1) }
                        { s := add(s, f(i)) }

                        function f(i) -> k {
                            let foo := 1
                            let bar := 2

                            if iszero(mod(i,3)) {
                                k := or(k,foo)
                            }
                            if iszero(mod(i,5)) {
                                k := or(k,bar)
                            }
                        }
                    }
                }
            }
        }
    }

    // this one is the same as above
    // used to trace each function's result
    {
        function a() -> r {
            r := add(b(), 1)

        }
        function b() -> x {
            let p, q := c()
            x := add(p, q)
        }
        function c() -> m, n {
            m := mul(d(), e())
            n := div(d(), e())
        }
        function d() -> s {
            for { let i := 0 } lt(i, 100) { i := add(i, 1) }
            { s := add(s, i) }
        }

        function e() -> s {
            for { let i := 0 } lt(i, 100) { i := add(i, 1) }
            { s := add(s, f(i)) }
        }
        function f(i) -> k {
            let foo := 1
            let bar := 2

            if iszero(mod(i,3)) {
                k := or(k,foo)
            }
            if iszero(mod(i,5)) {
                k := or(k,bar)
            }
        }
    }
}

// ----
// step: constantFunctionEvaluator
//
// {
//     {
//         function a() -> r
//         { r := 366367 }
//     }
//     {
//         function a_4() -> r_5
//         { r_5 := 366367 }
//         function b_6() -> x_7
//         { x_7 := 366366 }
//         function c_10() -> m_11, n_12
//         {
//             m_11 := 366300
//             n_12 := 66
//         }
//         function d_13() -> s_15
//         { s_15 := 4950 }
//         function e_14() -> s_17
//         { s_17 := 74 }
//         function f_19(i_20) -> k_21
//         {
//             let foo_22 := 1
//             let bar_23 := 2
//             if iszero(mod(i_20, 3)) { k_21 := or(k_21, foo_22) }
//             if iszero(mod(i_20, 5)) { k_21 := or(k_21, bar_23) }
//         }
//     }
// }
