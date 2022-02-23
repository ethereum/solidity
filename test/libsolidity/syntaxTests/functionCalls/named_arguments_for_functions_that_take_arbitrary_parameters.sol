contract C {
    function f() pure public {
        abi.encodeWithSelector({selector:"abc"});
    }
}
// ----
// TypeError 2627: (52-92): Named arguments cannot be used for functions that take arbitrary parameters.
