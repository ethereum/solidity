// this mainly tests the break and continue keyword
{
    function slow_gcd(a, b) -> res
    {
        for { res := a } gt(res, 1) { res := sub(res, 1) }
        {
            if mod(a, res) { continue }
            if mod(b, res) { continue }
            break
        }
    }

    function c1() -> r { r := slow_gcd(6, 9) }
    function c2() -> r { r := slow_gcd(10, 15) }
    function c3() -> r { r := slow_gcd(4, 2) }
    function c4() -> r { r := slow_gcd(2, 4) }
    function c5() -> r { r := slow_gcd(7, 6) }
    function c6() -> r { r := slow_gcd(99, 132) }
}
// ----
// step: constantFunctionEvaluator
//
// {
//     function slow_gcd(a, b) -> res
//     {
//         for { res := a } gt(res, 1) { res := sub(res, 1) }
//         {
//             if mod(a, res) { continue }
//             if mod(b, res) { continue }
//             break
//         }
//     }
//     function c1() -> r
//     { r := 3 }
//     function c2() -> r_1
//     { r_1 := 5 }
//     function c3() -> r_2
//     { r_2 := 2 }
//     function c4() -> r_3
//     { r_3 := 2 }
//     function c5() -> r_4
//     { r_4 := 1 }
//     function c6() -> r_5
//     { r_5 := 33 }
// }
