error MyCustomError(uint, bool);
error MyCustomError2(uint, bool);
error MyCustomError3(uint, bool, bool);

contract C {
    function f() pure public {
        true ? MyCustomError : MyCustomError;
        true ? MyCustomError : MyCustomError2;
        true ? MyCustomError : MyCustomError3;
        true ? MyCustomError : true;
        true ? true : MyCustomError;
    }
}

// ----
// TypeError 1080: (253-290): True expression's type error MyCustomError(uint256,bool) does not match false expression's type error MyCustomError3(uint256,bool,bool).
// TypeError 1080: (300-327): True expression's type error MyCustomError(uint256,bool) does not match false expression's type bool.
// TypeError 1080: (337-364): True expression's type bool does not match false expression's type error MyCustomError(uint256,bool).
