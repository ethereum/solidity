contract C {
    function() external returns (uint) { }
}
// ----
// TypeError: (45-51): Fallback function cannot return values.
