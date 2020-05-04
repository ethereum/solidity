struct s1 { s2 x; }
struct s2 { s1 y; }

contract C {
    // whatever
}
// ----
// TypeError: (0-19): Recursive struct definition.
