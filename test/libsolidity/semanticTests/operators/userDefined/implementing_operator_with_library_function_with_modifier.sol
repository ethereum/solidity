library L {
    type Int is int128;

    error DivisionByZero();

    modifier nonZero(Int a) {
        if (Int.unwrap(a) == 0)
            revert("Division by zero");
        _;
    }

    function div(Int a, Int b) pure public nonZero(b) returns (Int) {
        return Int.wrap(Int.unwrap(a) / Int.unwrap(b));
    }
}

contract C {
    using {L.div as /} for L.Int;

    function testDiv(L.Int a, L.Int b) pure public returns (L.Int) {
        return a / b;
    }
}
// ----
// testDiv(int128,int128): 10, 2 -> 5
// testDiv(int128,int128): 10, 0 -> FAILURE, hex"08c379a0", 0x20, 0x10, "Division by zero"
