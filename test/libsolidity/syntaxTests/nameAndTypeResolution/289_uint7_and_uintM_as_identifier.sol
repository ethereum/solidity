contract test {
string uintM = "Hello 4 you";
    function f() public {
        uint8 uint7 = 3;
        uint7 = 5;
        string memory intM;
        uint bytesM = 21;
        intM; bytesM;
    }
}
// ----
// Warning: (50-197): Function state mutability can be restricted to pure
