contract C {
    function f() view public {
        assembly {
            address
            pop
        }
    }
}
// ----
// SyntaxError: (75-82): The use of non-functional instructions is deprecated. Please use functional notation instead.
// SyntaxError: (95-98): The use of non-functional instructions is deprecated. Please use functional notation instead.
