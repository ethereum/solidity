contract C {
    modifier m { _; }
    modifier mv virtual { _; }
}

abstract contract A {
    modifier m { _; }
    modifier mv virtual { _; }
    modifier muv virtual;
}
// ----
