library L {
    function g(uint[]) internal pure {}
}
// ----
// TypeError: (27-33): Storage location must be one of: "storage", "memory" for parameter in internal function, but none was given.