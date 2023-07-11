type Int is int16;

function add(Int, Int) pure returns (Int) {}

contract C {
    using {add as +} for Int global;

    function test() pure public {
        Int.wrap(0) + Int.wrap(0);
    }
}
// ----
// SyntaxError 3367: (83-115): "global" can only be used at file level.
