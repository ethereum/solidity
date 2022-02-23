contract Test {

  struct Sample { bool flag; }
  Sample public s;

  modifier checkAddr(address _a){
      require(_a!=address(0));
      _;
  }
  function testFunc(address _a) external checkAddr(_a) {
        Sample storage t;
        t.flag=true;
    }
}
// ----
// TypeError 3464: (237-238): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
