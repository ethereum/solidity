contract test {
    modifier mod() { _; }

    function f() public {
        mod g;
    }
}
// ----
// TypeError: (77-80): Name has to refer to a struct, enum or contract.
