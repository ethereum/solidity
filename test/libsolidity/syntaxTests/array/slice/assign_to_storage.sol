contract c {
  bytes public b;
  function f() public {
    b = msg.data[:];
  }
}
// ----
