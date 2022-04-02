interface I {}
contract C {
    function f() public {
        new I();
    }
}
// ----
// TypeError 2971: (62-67='new I'): Cannot instantiate an interface.
