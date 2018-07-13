// This used to crash with some compiler versions.
contract SomeContract {

  uint public balance = 0;

  function balance(uint number) public {}

  function doSomething() public {
    balance(3);
  }
}
// ----
// DeclarationError: (106-145): Identifier already declared.
// TypeError: (185-195): Type is not callable
