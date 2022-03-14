contract C {
    using {f, g} for uint;

    function f(uint) internal { }
    function g(uint) public { }
}
// ----
// TypeError 4167: (24-25): Only file-level functions and library functions can be bound to a type in a "using" statement
// TypeError 4167: (27-28): Only file-level functions and library functions can be bound to a type in a "using" statement
