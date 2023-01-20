type Int is int;

contract C {
    using {add as +} for Int;

    function add(Int, Int) public pure returns (Int) {}
}
// ----
// TypeError 4167: (42-45): Only file-level functions and library functions can be attached to a type in a "using" statement
