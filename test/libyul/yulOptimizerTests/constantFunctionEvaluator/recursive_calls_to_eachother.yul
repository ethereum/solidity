/* python program to double check
def a1(x):
    if x > 0:
        return a2(x - 1) + 1
    return 0

def a2(x):
    if x > 0:
        return 2 * a3(x - 1)
    return 0

def a3(x):
    if x > 0:
        if x < 5:
            return a1(x - 1)
        return a2(x - 1)
    return 0

print(a1(10))
print(a2(10))
print(a3(10))
*/

{
    function a1(x) -> r {
        if x {
            r := add(1, a2(sub(x, 1)))
        }
    }
    function a2(x) -> r {
        if x {
            r := mul(2, a3(sub(x, 1)))
        }
    }
    function a3(x) -> r {
        if x {
            switch lt(x, 5)
            case 1 { r := a1(sub(x, 1)) }
            default { r := a2(sub(x, 1)) }
        }
    }
    function c1() -> r {
        r := a1(10)
    }
    function c2() -> r {
        r := a2(10)
    }
    function c3() -> r {
        r := a3(10)
    }
}
// ----
// step: constantFunctionEvaluator
//
// {
//     function a1(x) -> r
//     {
//         if x { r := add(1, a2(sub(x, 1))) }
//     }
//     function a2(x_1) -> r_2
//     {
//         if x_1
//         {
//             r_2 := mul(2, a3(sub(x_1, 1)))
//         }
//     }
//     function a3(x_3) -> r_4
//     {
//         if x_3
//         {
//             switch lt(x_3, 5)
//             case 1 { r_4 := a1(sub(x_3, 1)) }
//             default { r_4 := a2(sub(x_3, 1)) }
//         }
//     }
//     function c1() -> r_5
//     { r_5 := 9 }
//     function c2() -> r_6
//     { r_6 := 16 }
//     function c3() -> r_7
//     { r_7 := 8 }
// }
