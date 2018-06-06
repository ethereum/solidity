contract test {
    mapping(ufixed8x1 => string) fixedString;
    function f() public {
        fixedString[0.5] = "Half";
    }
}
