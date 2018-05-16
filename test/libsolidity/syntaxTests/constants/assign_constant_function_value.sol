contract C {
    function () pure returns (uint) x;
    uint constant y = x();
}
// ----
// Warning: (74-77): Initial value for constant variable has to be compile-time constant. This will fail to compile with the next breaking version change.
