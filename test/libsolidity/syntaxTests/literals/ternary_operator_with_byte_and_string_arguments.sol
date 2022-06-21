contract C {
    function f(bool cond) public {
         // OK
         string memory v1 = cond ? "foo" : string("bar");
         bytes1 v2 = cond ? bytes1("a") : "a";
         bytes2 v3 = cond ? bytes2("a") : "a";
         bytes2 v4 = cond ? bytes1("a") : bytes2("ab");

         // Errors
         cond ? bytes1("a") : string("b");
         cond ? 1 : bytes1("a");
    }
}
// ----
// TypeError 1080: (300-332): True expression's type bytes1 does not match false expression's type string memory.
// TypeError 1080: (343-365): True expression's type uint8 does not match false expression's type bytes1.
