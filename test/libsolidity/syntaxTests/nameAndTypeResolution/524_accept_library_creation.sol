library L {}
contract C {
    function f() public {
        new L();
    }
}
// ----
// TypeError 8696: (60-65): Cannot instantiate a library.
