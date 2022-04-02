contract C {
  struct S {uint x;}
  constructor(S) {}
}
// ----
// TypeError 6651: (48-49='S'): Data location must be "storage" or "memory" for constructor parameter, but none was given.
