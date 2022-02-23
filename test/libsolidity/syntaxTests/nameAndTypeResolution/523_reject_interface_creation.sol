interface I {}
contract C {
    function f() public {
        new I();
    }
}
// ----
// TypeError 2971: (62-67): Cannot instantiate an interface.
