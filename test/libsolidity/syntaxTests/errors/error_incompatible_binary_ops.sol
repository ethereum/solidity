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
// TypeError 2271: (86-116): Built-in binary operator << cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (126-156): Built-in binary operator >> cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (166-195): Built-in binary operator ^ cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (205-234): Built-in binary operator | cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (244-273): Built-in binary operator & cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (284-313): Built-in binary operator * cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (323-352): Built-in binary operator / cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (362-391): Built-in binary operator % cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (401-430): Built-in binary operator + cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (440-469): Built-in binary operator - cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (480-510): Built-in binary operator == cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (520-550): Built-in binary operator != cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (560-590): Built-in binary operator >= cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (600-630): Built-in binary operator <= cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (640-669): Built-in binary operator < cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (679-708): Built-in binary operator > cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (719-749): Built-in binary operator || cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
// TypeError 2271: (759-789): Built-in binary operator && cannot be applied to types error MyCustomError(uint256,bool) and error MyCustomError(uint256,bool).
