contract Test {

  struct Sample { bool flag; }
  Sample public s;

   function testFunc() external {
        if(true){}
        Sample storage t;
        t.flag=true;
    }
}
// ----
// TypeError 3464: (155-156): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
