contract test {
    struct Person {
        mapping(uint phone => uint[] calls) friends;
    }
}
// ----
