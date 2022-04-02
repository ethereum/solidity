contract C {
    enum Test { One, Two }
    function f() public {
        Test(-1);
        Test(2);
        Test(13);
        Test(5/3);
        Test(0.5);
    }
}
// ----
// TypeError 9640: (74-82='Test(-1)'): Explicit type conversion not allowed from "int_const -1" to "enum C.Test".
// TypeError 9640: (92-99='Test(2)'): Explicit type conversion not allowed from "int_const 2" to "enum C.Test".
// TypeError 9640: (109-117='Test(13)'): Explicit type conversion not allowed from "int_const 13" to "enum C.Test".
// TypeError 9640: (127-136='Test(5/3)'): Explicit type conversion not allowed from "rational_const 5 / 3" to "enum C.Test".
// TypeError 9640: (146-155='Test(0.5)'): Explicit type conversion not allowed from "rational_const 1 / 2" to "enum C.Test".
