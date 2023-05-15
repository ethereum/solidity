==== Source: a ====

/**This contract is empty*/ contract C {}

==== Source: b ====

/**This contract is empty
 and has a line-breaking comment.*/
contract C {}

==== Source: c ====

contract C {
  /** Some comment on state var.*/ uint public state;
  /** Some comment on Evt.*/ event Evt();
  /** Some comment on mod.*/ modifier mod() { _; }
  /** Some comment on fn.*/ function fn() public {}
}
// ----
