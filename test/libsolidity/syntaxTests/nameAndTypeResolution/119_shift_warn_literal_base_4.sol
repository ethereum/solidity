contract test {
    function f() pure public returns(uint) {
         uint8 x = 100;
         return 10 >> x;
    }
}
