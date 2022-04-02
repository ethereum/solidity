library test {
    modifier m virtual;
    function f() m public {
    }
}
// ----
// TypeError 3275: (19-38='modifier m virtual;'): Modifiers in a library cannot be virtual.
