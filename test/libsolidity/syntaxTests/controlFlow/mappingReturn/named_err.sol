contract C {
    function f() internal pure returns (mapping(uint=>uint) storage r) { }
}
// ----
// TypeError: (53-82): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
