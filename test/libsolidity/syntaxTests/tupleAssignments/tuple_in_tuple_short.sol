contract C {
  function f() public pure {
    int a;
    (((a,),)) = ((1,2),3);
  }
}
