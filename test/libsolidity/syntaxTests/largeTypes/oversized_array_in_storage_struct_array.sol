contract C {
    struct S { uint64[340282366920938463463374607431768211458][1701411834604692317316873037158841057281] x; }
    S[] p;
}
// ----
// TypeError 1534: (127-132): Type too large for storage.
