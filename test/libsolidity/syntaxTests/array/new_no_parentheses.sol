contract C {
    function f(uint size) public {
        new uint[1];
    }
}
// ----
// TypeError 3904: (60-67='uint[1]'): Length has to be placed in parentheses after the array type for new expression.
