contract C {
    function f(int a, int b) public pure returns (int) {
        return a % b; // can div by 0
    }
    function g(bool _check) public pure returns (int) {
        int x = type(int).min;
        if (_check) {
            return x / -1; // can overflow
        } else {
            unchecked { return x / -1; } // overflow not reported
        }
    }
}
// ====
// SMTEngine: all
// ----
// Warning 4281: (85-90): CHC: Division by zero happens here.
// Warning 4984: (242-248): CHC: Overflow (resulting value larger than 2**255 - 1) happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
