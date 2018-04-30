contract C {
  function f() {
    var a = "";
    bytes1 b = bytes1(a);
    bytes memory c = bytes(a);
    string memory d = string(a);
  }
}
// ----
// Warning: (34-39): Use of the "var" keyword is deprecated.
// TypeError: (61-70): Explicit type conversion not allowed from "string memory" to "bytes1".
