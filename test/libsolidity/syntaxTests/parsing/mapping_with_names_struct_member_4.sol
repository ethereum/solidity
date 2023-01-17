contract test {
    struct Person {
        mapping(uint phone => mapping(uint call => uint time) callTimes) friends;
    }
}
// ----
