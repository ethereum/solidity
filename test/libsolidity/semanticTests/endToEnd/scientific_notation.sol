contract C {
    function f() public returns(uint) {
        return 2e10 wei;
    }

    function g() public returns(uint) {
        return 200e-2 wei;
    }

    function h() public returns(uint) {
        return 2.5e1;
    }

    function i() public returns(int) {
        return -2e10;
    }

    function j() public returns(int) {
        return -200e-2;
    }

    function k() public returns(int) {
        return -2.5e1;
    }
}

// ====
// compileViaYul: also
// ----
// f() -> 20000000000
// f():"" -> "20000000000"
// g() -> 2
// g():"" -> "2"
// h() -> 25
// h():"" -> "25"
// i() -> -20000000000
// i():"" -> "115792089237316195423570985008687907853269984665640564039457584007893129639936"
// j() -> -2
// j():"" -> "115792089237316195423570985008687907853269984665640564039457584007913129639934"
// k() -> -25
// k():"" -> "115792089237316195423570985008687907853269984665640564039457584007913129639911"
