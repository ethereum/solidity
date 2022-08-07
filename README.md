# 面向合约的Solidity编程语言

[![Matrix Chat](https://img.shields.io/badge/Matrix%20-chat-brightgreen?style=plastic&logo=matrix)](https://matrix.to/#/#ethereum_solidity:gitter.im)
[![Gitter Chat](https://img.shields.io/badge/Gitter%20-chat-brightgreen?style=plastic&logo=gitter)](https://gitter.im/ethereum/solidity)
[![Solidity Forum](https://img.shields.io/badge/Solidity_Forum%20-discuss-brightgreen?style=plastic&logo=discourse)](https://forum.soliditylang.org/)
[![Twitter Follow](https://img.shields.io/twitter/follow/solidity_lang?style=plastic&logo=twitter)](https://twitter.com/solidity_lang)
[![Mastodon Follow](https://img.shields.io/mastodon/follow/000335908?domain=https%3A%2F%2Ffosstodon.org%2F&logo=mastodon&style=plastic)](https://fosstodon.org/@solidity)

您可以在Gitter和Matrix上与我们交谈，在Twitter上向我们发推特，或者在Solidity论坛上创建一个新的主题。
我们欢迎一切问题，反馈和建议。

Solidity是一种静态类型的，面向合约的高级语言，用于在Ethereum平台上实现智能合约。


为了获得一个好的概述和起点，请查看官方的 [Solidity语言门户](https://soliditylang.org)。

## 目录

- [背景介绍](#背景介绍)
- [构建和安装](#构建和安装)
- [示例](#示例)
- [文档](#文档)
- [发展](#发展)
- [维护者](#维护者)
- [许可](#许可)
- [安全](#安全)

## 背景介绍

Solidity是一种静态类型的大括号编程语言，用于开发在以太坊虚拟机上运行的智能合约。
智能合约是在点对点网络内执行的程序，没有人对执行有特别的权限，
因此它们允许实现价值代币，所有权，投票和其他种类的逻辑。

当部署合约时，您应该使用最新发布的 Solidity 版本。
这是因为重大的变化，以及新的功能和错误修复都是定期引入的。
我们目前使用 0.x 版本号 [以表示这种快速变化的节奏](https://semver.org/#spec-item-4)。

## 构建和安装

关于如何构建和安装 Solidity 编译器的说明可以在
[Solidity 文档](https://docs.soliditylang.org/en/latest/installing-solidity.html#building-from-source)
中找到。


## 示例

在 Solidity 中的 “Hello World” 程序比其他语言更没有用处，但此处仍然以此来展示：

```solidity
// SPDX-License-Identifier: MIT
pragma solidity >=0.6.0 <0.9.0;

contract HelloWorld {
    function helloWorld() external pure returns (string memory) {
        return "Hello, World!";
    }
}
```

要开始使用 Solidity，您可以使用 [Remix](https://remix.ethereum.org/)，
它是一个基于浏览器的IDE。这里有一些合约的例子：

1. [投票合约](https://docs.soliditylang.org/en/latest/solidity-by-example.html#voting)
2. [盲拍合约](https://docs.soliditylang.org/en/latest/solidity-by-example.html#blind-auction)
3. [安全的远程购买合约](https://docs.soliditylang.org/en/latest/solidity-by-example.html#safe-remote-purchase)
4. [微支付通道合约](https://docs.soliditylang.org/en/latest/solidity-by-example.html#micropayment-channel)

## 文档

Solidity 文档托管在 [阅读文档](https://docs.soliditylang.org) 。

## 发展

Solidity 仍在开发中。我们随时欢迎您的贡献!
如果您想提供帮助，请遵循 [开发者指南](https://docs.soliditylang.org/en/latest/contributing.html)。

您可以在 [项目管理](https://github.com/ethereum/solidity/projects)
中找到我们目前对即将发布的版本的功能和错误的优先级。


## 维护者
* [@axic](https://github.com/axic)
* [@chriseth](https://github.com/chriseth)

## 许可
Solidity 有 [GNU 通用公共许可证 v3.0](LICENSE.txt) 的许可。

一些第三方代码有其 [自己的许可条款](cmake/templates/license.h.in)。

## 安全

安全政策可以 [在这里找到](SECURITY.md)。
