library L {
    modifier mu;
    modifier muv virtual;
}
// ----
// TypeError 8063: (16-28='modifier mu;'): Modifiers without implementation must be marked virtual.
// TypeError 3275: (33-54='modifier muv virtual;'): Modifiers in a library cannot be virtual.
