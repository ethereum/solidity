pragma experimental "v0.5.0";
contract C {
    function f() view public {
        assembly {
            address
            pop
        }
    }
}
// ----
// SyntaxError: (105-112): The use of non-functional instructions is deprecated. Please use functional notation instead.
// SyntaxError: (125-128): The use of non-functional instructions is deprecated. Please use functional notation instead.
