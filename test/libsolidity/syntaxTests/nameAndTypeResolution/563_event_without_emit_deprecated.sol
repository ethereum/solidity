contract C {
    event e();
    function f() public {
        e();
    }
}
// ----
// Warning: (62-65): Invoking events without "emit" prefix is deprecated.
