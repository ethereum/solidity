type Int is int8;

contract C {
    function(Int, Int) external pure returns (Int) ptr;
    using {ptr as +} for Int;
}
// ----
// TypeError 8187: (99-102): Expected function name.
