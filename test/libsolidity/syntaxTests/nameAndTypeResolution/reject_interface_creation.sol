interface I {}
contract C {
    function f() public {
        new I();
    }
}
// ----
// TypeError: (62-67): Cannot instantiate an interface.
