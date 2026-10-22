

# Solidity Language Docs

## Local environment setup

### **Windows (PowerShell)**

1. Install Python: [https://www.python.org/downloads/](https://www.python.org/downloads/)
   (Check **"Add Python to PATH"** during installation)
2. Open PowerShell in the `/docs` folder:

   ```powershell
   cd docs
   ```
3. Create and activate a virtual environment:

   ```powershell
   py -m venv .venv
   .\.venv\Scripts\Activate.ps1
   ```
4. Install dependencies:

   ```powershell
   python -m pip install --upgrade pip
   if (Test-Path .\requirements.txt) { pip install -r .\requirements.txt } else { pip install sphinx pygments-lexer-solidity sphinx-rtd-theme }
   ```
5. Build the documentation:

   ```powershell
   python -m sphinx -b html . _build\html
   ```

---

### **macOS / Linux (bash)**

1. Install Python: [https://www.python.org/downloads/](https://www.python.org/downloads/)
2. Open a terminal in the `/docs` folder:

   ```bash
   cd /docs
   ```
3. Create and activate a virtual environment:

   ```bash
   python3 -m venv .venv
   source .venv/bin/activate
   ```
4. Install dependencies:

   ```bash
   python -m pip install --upgrade pip
   if [ -f requirements.txt ]; then pip install -r requirements.txt; else pip install sphinx pygments-lexer-solidity sphinx-rtd-theme; fi
   ```
5. Build the documentation:

   ```bash
   python -m sphinx -b html . _build/html
   ```

   *(Alternatively, you can run the provided script if available)*:

   ```bash
   chmod +x docs.sh
   ./docs.sh
   ```

---

## Serve environment

### **Windows (PowerShell)**

```powershell
python -m http.server 8080 --directory _build\html
```

### **macOS / Linux (bash)**

```bash
python3 -m http.server -d _build/html --cgi 8080
```

Visit the dev server at:
[http://localhost:8080](http://localhost:8080)

---
