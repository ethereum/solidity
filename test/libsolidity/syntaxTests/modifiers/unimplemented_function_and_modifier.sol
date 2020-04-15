abstract contract A {
  function foo() public virtual;
  function foo(uint x) virtual public returns(uint);
  modifier mod() virtual;
}

contract B is A {
  function foo(uint x) override public returns(uint) {return x;}
  modifier mod() override { _; }
}

contract C is A {
  function foo() public override {}
  modifier mod() override { _; }
}

contract D is A {
  function foo() public override {}
  function foo(uint x) override public returns(uint) {return x;}
}

/* No errors */
contract E is A {
  function foo() public override {}
  function foo(uint x) override public returns(uint) {return x;}
  modifier mod() override { _;}
}
// ----
// TypeError: (137-254): Contract "B" should be marked as abstract.
// TypeError: (256-344): Contract "C" should be marked as abstract.
// TypeError: (346-466): Contract "D" should be marked as abstract.
