{
    // with side effect
    {
        function a(x) -> r {
            r := b(x)
        }

        function b(x) -> r {
            r := c(x)
        }

        function c(x) -> r {
            r := d(x)
        }

        function d(x) -> r {
            mstore(x, 1)
            r := add(x, 1)
        }

        function test() -> k {
            k := a(10)
        }
    }
    // without side effect
    {
        function a(x) -> r {
            r := b(x)
        }

        function b(x) -> r {
            r := c(x)
        }

        function c(x) -> r {
            r := d(x)
        }

        function d(x) -> r {
            r := add(x, 1)
        }

        function test() -> k {
            k := a(10)
        }
    }
}
// ----
// step: constantFunctionEvaluator
//
// {
//     {
//         function a(x) -> r
//         { r := b(x) }
//         function b(x_1) -> r_2
//         { r_2 := c(x_1) }
//         function c(x_3) -> r_4
//         { r_4 := d(x_3) }
//         function d(x_5) -> r_6
//         {
//             mstore(x_5, 1)
//             r_6 := add(x_5, 1)
//         }
//         function test() -> k
//         { k := a(10) }
//     }
//     {
//         function a_7(x_8) -> r_9
//         { r_9 := b_10(x_8) }
//         function b_10(x_11) -> r_12
//         { r_12 := c_13(x_11) }
//         function c_13(x_14) -> r_15
//         { r_15 := d_16(x_14) }
//         function d_16(x_17) -> r_18
//         { r_18 := add(x_17, 1) }
//         function test_19() -> k_20
//         { k_20 := 11 }
//     }
// }
