contract test {
    modifier mod() { _; }

    function f() public {
        mod  ;
    }
}
// ----
// TypeError 3112: (77-80): Modifier can only be referenced in function headers.
