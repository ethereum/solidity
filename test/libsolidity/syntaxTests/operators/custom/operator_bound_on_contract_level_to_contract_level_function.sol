type Int is int;

contract C {
    using {add as +} for Int;

    function add(Int, Int) public returns (Int) {
        return 0;
    }
}

// ----
// TypeError 4167: (42-45): Only file-level functions and library functions can be bound to a type in a "using" statement
