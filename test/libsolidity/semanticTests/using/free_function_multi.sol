contract C {
    function f(uint z) pure external returns(uint) {
        return z.id();
    }

    using {id, zero, zero, id} for uint;

    function g(uint z) pure external returns (uint) {
        return z.zero();
    }
}

function id(uint x) pure returns (uint) {
    return x;
}

function zero(uint) pure returns (uint) {
    return 0;
}

// ====
// compileViaYul: also
// ----
// f(uint256): 10 -> 10
// g(uint256): 10 -> 0
// f(uint256): 256 -> 0x0100
// g(uint256): 256 -> 0
