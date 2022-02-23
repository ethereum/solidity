contract C {
  int[0] a;
  uint[0] b;
  bytes1[0] c;
  bytes32[0] d;
  bytes[0] e;
  string[0] f;
}
// ----
// TypeError 1406: (19-20): Array with zero length specified.
// TypeError 1406: (32-33): Array with zero length specified.
// TypeError 1406: (47-48): Array with zero length specified.
// TypeError 1406: (63-64): Array with zero length specified.
// TypeError 1406: (77-78): Array with zero length specified.
// TypeError 1406: (92-93): Array with zero length specified.
