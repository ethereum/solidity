library L {
    modifier mv virtual { _; }
}
// ----
// TypeError 3275: (16-42='modifier mv virtual { _; }'): Modifiers in a library cannot be virtual.
