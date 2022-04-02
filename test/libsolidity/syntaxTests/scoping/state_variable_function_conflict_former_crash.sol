// This used to crash with some compiler versions.
contract SomeContract {

  uint public balance = 0;

  function balance(uint number) public {}

  function doSomething() public {
    balance(3);
  }
}
// ----
// DeclarationError 2333: (106-145): Identifier already declared.
// TypeError 5704: (185-195='balance(3)'): Type is not callable
