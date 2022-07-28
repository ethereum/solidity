type Int is int8;

contract C {
    function(Int, Int) external returns (Int) ptr;
    using {ptr as +} for Int;
}
// ----
// TypeError 8187: (94-97): Expected function name.
