// The constructor of a base class should not be visible in the derived class
contract A { function A(string memory s) public { } }
contract B is A {
  function f() pure public {
    A x = A(0); // convert from address
    string memory y = "ab";
    A(y); // call as a function is invalid
    x;
  }
}
// ----
// SyntaxError: (91-129): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
// TypeError: (251-255): Explicit type conversion not allowed from "string memory" to "contract A".
