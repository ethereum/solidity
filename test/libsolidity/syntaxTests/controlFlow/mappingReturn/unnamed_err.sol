contract C {
    function f() internal pure returns (mapping(uint=>uint) storage) {}
}
// ----
// TypeError: (53-72): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
