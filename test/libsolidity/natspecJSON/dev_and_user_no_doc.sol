contract test {
    function mul(uint a) public returns (uint d) {
        return a * 7;
    }
    function sub(int input) public returns (int d) {
        return input - 3;
    }
}

// ----
// ----
// :test devdoc
// {
//     "kind": "dev",
//     "methods": {},
//     "version": 1
// }
//
// :test userdoc
// {
//     "kind": "user",
//     "methods": {},
//     "version": 1
// }
