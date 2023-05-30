contract C { constructor(uint) {} }
contract D is C {
  uint immutable t;
  constructor() C(t=2) {}
}
