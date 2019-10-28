contract test {
    modifier mod() { _; }

    function f() public {
        mod  ;
    }
}
// ----
// TypeError: (77-80): Modifier can only be referenced in function headers.
