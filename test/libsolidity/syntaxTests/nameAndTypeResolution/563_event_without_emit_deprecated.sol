contract C {
    event e();
    function f() public {
        e();
    }
}
// ----
// TypeError: (62-65): Event invocations have to be prefixed by "emit".
