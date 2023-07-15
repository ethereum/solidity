# Solidity Language Docs

## Local environment setup

1. Install python https://www.python.org/downloads/
1. Install sphinx (the tool used to generate the docs) https://www.sphinx-doc.org/en/master/usage/installation.html

Go to `/docs` and run `./docs.sh` to install dependencies and build the project:

```sh
cd docs
./docs.sh
```

That will output the generated htmls under _build/

## Serve environment

```py
python3 -m http.server -d _build/html --cgi 8080
```

Visit dev server at http://localhost:8080
