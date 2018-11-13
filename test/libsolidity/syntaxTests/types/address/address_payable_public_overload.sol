contract C {
    function f(address) public pure {}
    function f(address payable) public pure {}

}
// ----
// TypeError: (56-98): Function overload clash during conversion to external types for arguments.
