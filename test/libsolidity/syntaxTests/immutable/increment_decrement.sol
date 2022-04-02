contract C {
  uint immutable x;
  uint immutable y;
  constructor() {
    ++x;
    --y;
  }
}
// ----
// TypeError 3969: (77-78='x'): Immutable variables must be initialized using an assignment.
// TypeError 3969: (86-87='y'): Immutable variables must be initialized using an assignment.
