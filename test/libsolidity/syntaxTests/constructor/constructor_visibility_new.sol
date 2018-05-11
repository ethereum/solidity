// The constructor of a base class should not be visible in the derived class
contract A { constructor(string memory) public { } }
contract B is A {
  function f() pure public {
    A x = A(0); // convert from address
    string memory y = "ab";
    A(y); // call as a function is invalid
    x;
  }
}
// ----
// TypeError: (250-254): Explicit type conversion not allowed from "string memory" to "contract A".
