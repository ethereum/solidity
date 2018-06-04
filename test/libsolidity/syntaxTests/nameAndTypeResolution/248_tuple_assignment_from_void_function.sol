contract C {
    function f() public { }
    function g() public {
        var (x,) = (f(), f());
    }
}
// ----
// Warning: (80-81): Use of the "var" keyword is deprecated.
// Warning: (87-90): Tuple component cannot be empty.
// Warning: (92-95): Tuple component cannot be empty.
// TypeError: (80-81): Cannot declare variable with void (empty tuple) type.
