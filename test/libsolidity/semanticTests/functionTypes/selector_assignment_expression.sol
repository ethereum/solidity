contract C {
    bool public z;
    function f() public {
        ((z = true) ? this.f : this.f).selector;
    }
}

// ----
// f()
// z() -> true
