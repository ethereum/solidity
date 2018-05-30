contract C {
    function f() public returns (bytes4) {
        uint32 tmp = 1;
        return tmp;
    }
}
// ----
// TypeError: (95-98): Return argument type uint32 is not implicitly convertible to expected type (type of first return variable) bytes4.
