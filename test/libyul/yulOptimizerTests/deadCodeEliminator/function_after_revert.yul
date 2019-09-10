{
    fun()

    revert(0, 0)

    function fun()
    {
        return(1, 1)

        pop(sub(10, 5))
    }

    pop(add(1, 1))
}
// ====
// step: deadCodeEliminator
// ----
// {
//     fun()
//     revert(0, 0)
//     function fun()
//     { return(1, 1) }
// }
