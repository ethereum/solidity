contract C {
    function f() pure public {
        abi.decode("", ((uint, int)[5][6]));
    }
}
// ----
// TypeError 2614: (68-79): Indexed expression has to be a type, mapping or array (is tuple(type(uint256),type(int256)))
