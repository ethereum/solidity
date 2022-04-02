contract C {
    function x(bool) public {}
    function y() public {}

    function f() public {
        true ? x : y;
    }
}
// ----
// TypeError 1080: (106-118='true ? x : y'): True expression's type function (bool) does not match false expression's type function ().
