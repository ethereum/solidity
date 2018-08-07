contract C {
    function f(uint[] storage a) external {}
}
// ----
// TypeError: (28-44): Storage location must be none or "calldata" for parameter in external function, but "storage" was given.
