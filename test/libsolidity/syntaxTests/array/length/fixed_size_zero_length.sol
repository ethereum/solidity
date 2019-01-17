contract C {
  int[0] a;
  uint[0] b;
  byte[0] c;
  bytes32[0] d;
  bytes[0] e;
  string[0] f;
}
// ----
// TypeError: (19-20): Array with zero length specified.
// TypeError: (32-33): Array with zero length specified.
// TypeError: (45-46): Array with zero length specified.
// TypeError: (61-62): Array with zero length specified.
// TypeError: (75-76): Array with zero length specified.
// TypeError: (90-91): Array with zero length specified.
