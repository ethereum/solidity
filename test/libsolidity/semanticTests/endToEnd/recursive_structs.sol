contract C {
    struct S {
        S[] x;
    }
    S sstorage;

    function f() public returns(uint) {
        S memory s;
        s.x = new S[](10);
        delete s;
        // TODO Uncomment after implemented.
        // sstorage.x.push();
        delete sstorage;
        return 1;
    }
}

// ----
// f() -> 1
// f():"" -> "1"
