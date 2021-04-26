contract C {
    bytes s;
    function f(bytes calldata c, string memory m) public view returns (bytes3 r1, bytes16 r2, bytes32 r3) {
        require(c.length >= 3, "");
        r2 = s;
        r1 = c[0:3];
        r3 = bytes32(m);
        r3 = m;
    }
}
// ----
// TypeError 7407: (183-184): Type bytes storage ref is not implicitly convertible to expected type bytes16.
// TypeError 7407: (199-205): Type bytes calldata slice is not implicitly convertible to expected type bytes3.
// TypeError 9640: (220-230): Explicit type conversion not allowed from "string memory" to "bytes32".
// TypeError 7407: (245-246): Type string memory is not implicitly convertible to expected type bytes32.
