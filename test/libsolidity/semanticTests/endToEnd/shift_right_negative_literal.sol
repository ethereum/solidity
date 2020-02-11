contract C {
    function f1() public pure returns(bool) {
        return (-4266 >> 0) == -4266;
    }

    function f2() public pure returns(bool) {
        return (-4266 >> 1) == -2133;
    }

    function f3() public pure returns(bool) {
        return (-4266 >> 4) == -267;
    }

    function f4() public pure returns(bool) {
        return (-4266 >> 8) == -17;
    }

    function f5() public pure returns(bool) {
        return (-4266 >> 16) == -1;
    }

    function f6() public pure returns(bool) {
        return (-4266 >> 17) == -1;
    }

    function g1() public pure returns(bool) {
        return (-4267 >> 0) == -4267;
    }

    function g2() public pure returns(bool) {
        return (-4267 >> 1) == -2134;
    }

    function g3() public pure returns(bool) {
        return (-4267 >> 4) == -267;
    }

    function g4() public pure returns(bool) {
        return (-4267 >> 8) == -17;
    }

    function g5() public pure returns(bool) {
        return (-4267 >> 16) == -1;
    }

    function g6() public pure returns(bool) {
        return (-4267 >> 17) == -1;
    }
}

// ====
// compileViaYul: also
// ----
// f1() -> true
// f1():"" -> "1"
// f2() -> true
// f2():"" -> "1"
// f3() -> true
// f3():"" -> "1"
// f4() -> true
// f4():"" -> "1"
// f5() -> true
// f5():"" -> "1"
// f6() -> true
// f6():"" -> "1"
// g1() -> true
// g1():"" -> "1"
// g2() -> true
// g2():"" -> "1"
// g3() -> true
// g3():"" -> "1"
// g4() -> true
// g4():"" -> "1"
// g5() -> true
// g5():"" -> "1"
// g6() -> true
// g6():"" -> "1"
