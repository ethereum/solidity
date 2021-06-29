contract C {
    modifier mu;
    modifier muv virtual;
}
// ----
// TypeError 3656: (0-57): Contract "C" should be marked as abstract.
// TypeError 8063: (17-29): Modifiers without implementation must be marked virtual.
