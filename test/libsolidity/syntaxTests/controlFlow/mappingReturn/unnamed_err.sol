contract C {
    function f() internal pure returns (mapping(uint=>uint) storage) {}
}
// ----
// TypeError: (53-80): This variable is of storage pointer type and can be returned without prior assignment.
