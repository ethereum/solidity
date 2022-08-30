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
// TypeError 2271: (86-116): Operator << not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (126-156): Operator >> not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (166-195): Operator ^ not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (205-234): Operator | not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (244-273): Operator & not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (284-313): Operator * not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (323-352): Operator / not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (362-391): Operator % not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (401-430): Operator + not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (440-469): Operator - not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (480-510): Operator == not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (520-550): Operator != not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (560-590): Operator >= not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (600-630): Operator <= not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (640-669): Operator < not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (679-708): Operator > not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (719-749): Operator || not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (759-789): Operator && not compatible with types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
