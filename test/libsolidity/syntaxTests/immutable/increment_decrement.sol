contract C {
  uint immutable x;
  uint immutable y;
  constructor() {
    ++x;
    --y;
  }
}
// ----
// TypeError 7733: (77-78): Immutable variables cannot be read during contract creation time, which means they cannot be read in the constructor or any function or modifier called from it.
// TypeError 7733: (86-87): Immutable variables cannot be read during contract creation time, which means they cannot be read in the constructor or any function or modifier called from it.
