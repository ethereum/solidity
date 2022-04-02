contract C {
    function f() public {
        true ? true : 2;
    }
}
// ----
// TypeError 1080: (47-62='true ? true : 2'): True expression's type bool does not match false expression's type uint8.
