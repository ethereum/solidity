interface I {
    struct A {
        // This is currently expected to break, but it *may* change in the future.
        int dummy;
    }
}
// ----
// TypeError: (18-136): Structs cannot be defined in interfaces.
// TypeError: (120-129): Variables cannot be declared in interfaces.
