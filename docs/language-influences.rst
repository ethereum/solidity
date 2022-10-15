###################
语言的影响因素
###################

Solidity是一种 `花括号语言 <https://en.wikipedia.org/wiki/List_of_programming_languages_by_type#Curly-bracket_languages>`_，
受到几种著名编程语言的影响和启发。

Solidity受C++的影响最深，但也借用了Python，JavaScript等语言的概念。

从变量声明的语法，for循环，重载函数的概念，隐式和显式类型转换以及许多其他细节中可以看出C++的影响。

这是由于变量的函数级范围和关键字 ``var`` 的使用。
从0.4.0版本开始，JavaScript的影响已经减少。
现在，剩下的与JavaScript的主要相似之处是，使用关键字 ``function`` 来定义函数。
Solidity还支持导入语法和语义，这些都与JavaScript中的相似。
除了这些点，Solidity看起来和其他大多数花括号语言一样，不再有主要的JavaScript影响。

对Solidity的另一个影响是Python。
Solidity的修改器是为了模拟Python的装饰器而添加的，但其功能受到很大限制。
此外，多重继承，C3线性化和 ``super`` 关键字以及值和引用类型的一般赋值和复制语义都来自Python。
