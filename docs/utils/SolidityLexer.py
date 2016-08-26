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
            (r'@author|@dev|@notice|@return|@param|@why3|@title', Keyword),
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
            (r'(for|in|while|do|break|return|continue|switch|case|default|if|else|'
             r'throw|try|catch|finally|new|delete|typeof|instanceof|void|'
             r'this|import|mapping|returns|private|public|external|internal|'
             r'constant|memory|storage)\b', Keyword, 'slashstartsregex'),
            (r'(var|let|with|function|event|modifier|struct|enum|contract|library)\b', Keyword.Declaration, 'slashstartsregex'),
            (r'(bytes|string|address|uint|int|bool|byte|' +
             '|'.join(
                 ['uint%d' % (i + 8) for i in range(0, 256, 8)] +
                 ['int%d' % (i + 8) for i in range(0, 256, 8)] +
                 ['bytes%d' % (i + 1) for i in range(0, 32)] +
                 ['ufixed%dx%d' % ((i), (j + 8)) for i in range(0, 256, 8) for j in range(0, 256 - i, 8)] +
                 ['fixed%dx%d' % ((i), (j + 8)) for i in range(0, 256, 8) for j in range(0, 256 - i, 8)]
             ) + r')\b', Keyword.Type, 'slashstartsregex'),
            (r'(abstract|boolean|byte|char|class|const|debugger|double|enum|export|'
             r'extends|final|float|goto|implements|int|interface|long|native|'
             r'package|private|protected|public|short|static|super|synchronized|throws|'
             r'transient|volatile)\b', Keyword.Reserved),
            (r'(true|false|null|NaN|Infinity|undefined)\b', Keyword.Constant),
            (r'(Array|Boolean|Date|Error|Function|Math|netscape|'
             r'Number|Object|Packages|RegExp|String|sun|decodeURI|'
             r'decodeURIComponent|encodeURI|encodeURIComponent|'
             r'Error|eval|isFinite|isNaN|parseFloat|parseInt|document|this|'
             r'window)\b', Name.Builtin),
            (r'[$a-zA-Z_][a-zA-Z0-9_]*', Name.Other),
            (r'[0-9][0-9]*\.[0-9]+([eE][0-9]+)?[fd]?', Number.Float),
            (r'0x[0-9a-fA-F]+', Number.Hex),
            (r'[0-9]+', Number.Integer),
            (r'"(\\\\|\\"|[^"])*"', String.Double),
            (r"'(\\\\|\\'|[^'])*'", String.Single),
        ]
    }
