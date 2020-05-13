contract c {
  bytes public b;
  function f() public {
    b = msg.data[:];
  }
}
// ----
// TypeError: (63-74): Type bytes calldata slice is not implicitly convertible to expected type bytes storage ref.
