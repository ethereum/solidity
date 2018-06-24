contract C {
    function x(bool) public {}
    function y() public {}

    function f() public {
        true ? x : y;
    }
}
// ----
// TypeError: (106-118): True expression's type function (bool) doesn't match false expression's type function ().
