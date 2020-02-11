contract C {
    function f(int a, int b) public returns(int) {
        return a << b;
    }

    function g(int a, int b) public returns(int) {
        return a >> b;
    }
}

// ----
// f(int256,int256): 1), -1) -> 
// f(int256,int256):"1, 115792089237316195423570985008687907853269984665640564039457584007913129639935" -> ""
// g(int256,int256): 1), -1) -> 
// g(int256,int256):"1, 115792089237316195423570985008687907853269984665640564039457584007913129639935" -> ""
