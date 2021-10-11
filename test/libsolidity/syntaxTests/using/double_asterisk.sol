function id(uint x) pure returns (uint) {
    return x;
}

function zero(address) pure returns (address) {
    return address(0);
}

contract C {
    using * for *;
    function f(uint x) pure external returns (uint) {
        return x.id();
    }
    function g(address a) pure external returns (address) {
        return a.zero();
    }
}
// ----
// ParserError 2314: (156-157): Expected identifier but got '*'
