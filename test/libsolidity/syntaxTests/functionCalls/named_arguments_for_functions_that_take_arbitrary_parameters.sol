contract C {
    function f() pure public {
        abi.encodeWithSelector({selector:"abc"});
    }
}
// ----
// TypeError: (52-92): Named arguments cannot be used for functions that take arbitrary parameters.
