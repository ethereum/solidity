contract C {
    function f() view public {
        assembly {
            address
            pop
        }
    }
}
// ----
// Warning: (75-82): The use of non-functional instructions is deprecated. Please use functional notation instead.
// Warning: (95-98): The use of non-functional instructions is deprecated. Please use functional notation instead.
