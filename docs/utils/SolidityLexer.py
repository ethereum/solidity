# -*- coding: utf-8 -*-

import re
import copy

from pygments.lexer import RegexLexer, ExtendedRegexLexer, bygroups, using, \
     include, this
from pygments.token import Text, Comment, Operator, Keyword, Name, String, \
     Number, Other, Punctuation, Literal

__all__ = ['SolidityLexer']

class SolidityLexer(RegexLexer):
    name = "Solidity"
    aliases = ['sol', 'solidity']
    filenames = ['*.sol']
    mimetypes = []
    flags = re.DOTALL
    tokens = {
        'commentsandwhitespace': [
            (r'\s+', Text),
            (r'<!--', Comment),
            (r'///', Comment.Special, 'docstringsingle'),
            (r'//.*?\n', Comment.Single),
            (r'/\*\*', Comment.Special, 'docstringmulti'),
            (r'/\*.*?\*/', Comment.Multiline)
        ],
        'natspec': [
            (r'@author|@dev|@notice|@return|@param|@title', Keyword),
            (r'.[^@*\n]*?', Comment.Special)
        ],
        'docstringsingle': [
            (r'\n', Comment.Special, '#pop'),
            include('natspec')
        ],
        'docstringmulti': [
            (r'\*/', Comment.Special, '#pop'),
            include('natspec')
        ],
        'slashstartsregex': [
            include('commentsandwhitespace'),
            (r'/(\\.|[^[/\\\n]|\[(\\.|[^\]\\\n])*])+/'
             r'([gim]+\b|\B)', String.Regex, '#pop'),
            (r'(?=/)', Text, ('#pop', 'badregex')),
            (r'', Text, '#pop')
        ],
        'badregex': [
            (r'\n', Text, '#pop')
        ],
        'root': [
            (r'^(?=\s|/|<!--)', Text, 'slashstartsregex'),
            include('commentsandwhitespace'),
            (r'\+\+|--|\*\*|~|&&|\?|:|\|\||\\(?=\n)|'
             r'(<<|>>>?|==?|!=?|[-<>+*%&\|\^/])=?', Operator, 'slashstartsregex'),
            (r'[{(\[;,]', Punctuation, 'slashstartsregex'),
            (r'[})\].]', Punctuation),
            (r'(anonymous|as|assembly|break|constant|continue|do|delete|else|external|for|hex|if|'
             r'indexed|internal|import|is|mapping|memory|new|payable|public|pragma|'
             r'private|pure|return|returns|storage|super|this|throw|using|view|while)\b', Keyword, 'slashstartsregex'),
            (r'(var|function|event|modifier|struct|enum|contract|library|interface)\b', Keyword.Declaration, 'slashstartsregex'),
            (r'(bytes|string|address|uint|int|bool|byte|' +
             '|'.join(
                 ['uint%d' % (i + 8) for i in range(0, 256, 8)] +
                 ['int%d' % (i + 8) for i in range(0, 256, 8)] +
                 ['bytes%d' % (i + 1) for i in range(0, 32)] +
                 ['ufixed%dx%d' % ((i), (j + 8)) for i in range(0, 256, 8) for j in range(0, 256 - i, 8)] +
                 ['fixed%dx%d' % ((i), (j + 8)) for i in range(0, 256, 8) for j in range(0, 256 - i, 8)]
             ) + r')\b', Keyword.Type, 'slashstartsregex'),
            (r'(wei|szabo|finney|ether|seconds|minutes|hours|days|weeks|years)\b', Keyword.Type, 'slashstartsregex'),
            (r'(abstract|after|case|catch|default|final|in|inline|let|match|'
             r'null|of|relocatable|static|switch|try|type|typeof)\b', Keyword.Reserved),
            (r'(true|false)\b', Keyword.Constant),
            (r'(block|msg|tx|now|suicide|selfdestruct|addmod|mulmod|sha3|keccak256|log[0-4]|'
             r'sha256|ecrecover|ripemd160|assert|revert|require)', Name.Builtin),
            (r'[$a-zA-Z_][a-zA-Z0-9_]*', Name.Other),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?', Number.Float),
            (r'0x[0-9a-fA-F]+', Number.Hex),
            (r'[0-9]+([eE][0-9]+)?', Number.Integer),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r"'(\\\\|\\'|[^'])*'", String.Single),
        ]
    }
