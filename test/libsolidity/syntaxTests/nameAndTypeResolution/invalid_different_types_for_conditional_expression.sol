contract C {
    function f() public {
        true ? true : 2;
    }
}
// ----
// TypeError: (47-62): True expression's type bool doesn't match false expression's type uint8.
