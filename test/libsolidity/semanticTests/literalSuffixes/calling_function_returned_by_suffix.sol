function f(uint x) pure returns (uint) {
    return 21 * x;
}

function suffix(uint) pure suffix returns (function(uint) returns (uint)) {
    return f;
}

contract C {
    function test() public returns (uint) {
        return (1 suffix)(2) + 1 suffix(1);
    }
}
// ----
// test() -> 63
