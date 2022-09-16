type Int is int16;

function add(Int, Int) pure returns (Int) {
    return Int.wrap(0);
}

contract C {
    using {add as +} for Int global;

    function test() pure public {
        Int.wrap(0) + Int.wrap(0);
    }
}

// ----
// SyntaxError 3367: (108-140): "global" can only be used at file level.
