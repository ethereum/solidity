contract C {
    function f() internal {
        assembly {
            tstore(0, 0)
        }
    }
    function g() public view {
        function() internal ptr = f;
        function() internal view ptr2;
        assembly { ptr2 := ptr }
        ptr2(); // we force calling the non-view function, which should result in a revert during the staticcall
    }
    function test() public {
        this.g(); // an external call to a view function should use static call
    }
}
// ====
// EVMVersion: >=cancun
// ----
// test() -> FAILURE
// gas irOptimized: 98437877
// gas legacy: 98437871
// gas legacyOptimized: 98437872
