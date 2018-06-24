pragma experimental "v0.5.0";
contract C {
    event e();
    function f() public {
        e();
    }
}
// ----
// TypeError: (92-95): Event invocations have to be prefixed by "emit".
