contract C {
    function f(uint size) public {
        new uint[1];
    }
}
// ----
// TypeError: (60-67): Length has to be placed in parentheses after the array type for new expression.
