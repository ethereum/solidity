contract test {
    modifier mod() { _; }

    function f() public {
        mod g;
    }
}
// ----
// TypeError 8872: (77-80): Modifiers cannot be used as variable type.
