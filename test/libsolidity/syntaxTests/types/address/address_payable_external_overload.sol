contract C {
    function f(address) external pure {}
    function f(address payable) external pure {}

}
// ----
// TypeError: (58-102): Function overload clash during conversion to external types for arguments.
