error MyCustomError(uint, bool);

contract C {
    function f() pure public {
        MyCustomError << MyCustomError;
        MyCustomError >> MyCustomError;
        MyCustomError ^ MyCustomError;
        MyCustomError | MyCustomError;
        MyCustomError & MyCustomError;

        MyCustomError * MyCustomError;
        MyCustomError / MyCustomError;
        MyCustomError % MyCustomError;
        MyCustomError + MyCustomError;
        MyCustomError - MyCustomError;

        MyCustomError == MyCustomError;
        MyCustomError != MyCustomError;
        MyCustomError >= MyCustomError;
        MyCustomError <= MyCustomError;
        MyCustomError < MyCustomError;
        MyCustomError > MyCustomError;

        MyCustomError || MyCustomError;
        MyCustomError && MyCustomError;
    }
}

// ----
// TypeError 2271: (86-116): Binary operator << not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (126-156): Binary operator >> not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (166-195): Binary operator ^ not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (205-234): Binary operator | not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (244-273): Binary operator & not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (284-313): Binary operator * not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (323-352): Binary operator / not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (362-391): Binary operator % not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (401-430): Binary operator + not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (440-469): Binary operator - not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (480-510): Binary operator == not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (520-550): Binary operator != not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (560-590): Binary operator >= not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (600-630): Binary operator <= not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (640-669): Binary operator < not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (679-708): Binary operator > not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (719-749): Binary operator || not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (759-789): Binary operator && not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
