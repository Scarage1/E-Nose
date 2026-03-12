# Contributing to E-Nose

Thank you for your interest in contributing! All contributions are welcome — bug fixes, documentation improvements, new features, and dataset sharing.

---

## Ways to Contribute

| Type | How |
|------|-----|
| 🐛 Bug report | Open an [issue](https://github.com/Scarage1/E-Nose/issues/new?template=bug_report.md) |
| 💡 Feature idea | Open an [issue](https://github.com/Scarage1/E-Nose/issues/new?template=feature_request.md) |
| 📖 Docs improvement | Edit a `.md` file and open a PR |
| 🔧 Code improvement | Fork → branch → PR |
| 📊 Dataset sharing | See the datasets section below |

---

## Development Workflow

### 1 — Fork & Clone

```bash
git clone git@github.com:<your-username>/E-Nose.git
cd E-Nose
```

### 2 — Create a Branch

Use a descriptive name:

```bash
git checkout -b fix/bme680-i2c-address
git checkout -b feat/wifi-logging
git checkout -b docs/improve-training-guide
```

### 3 — Make Changes

- **Arduino / C++**: follow the existing code style (Doxygen header comments, `///` inline docs, `#define` constants in UPPER_SNAKE_CASE).
- **Python**: follow [PEP 8](https://peps.python.org/pep-0008/). Run `flake8 scripts/` before committing.
- **Docs**: use clear headings, tables for structured info, and code blocks with language hints.

### 4 — Commit

Use [Conventional Commits](https://www.conventionalcommits.org/) format:

```
feat: add Wi-Fi prediction logging
fix: correct BME680 I2C address comment
docs: expand sensor preheat guidance
refactor: simplify normalisation loop
```

### 5 — Open a Pull Request

Push your branch and open a PR against `main`. Fill in the PR template.

---

## Code Style Quick Reference

### Arduino (C++)

```cpp
/**
 * @brief  One-line description.
 * @param  param_name  Description.
 * @return Description.
 */
void myFunction(int param_name) {
  // Use 2-space indentation
}
```

### Python

```python
def my_function(param: str) -> None:
    """One-line docstring.

    Args:
        param: Description.
    """
```

---

## Sharing Datasets

Datasets are excluded from the repo by default (see `.gitignore`) because they are environment-specific. If you want to share a useful dataset:

1. Consider using [Git LFS](https://git-lfs.com/) or an external host (e.g., Google Drive, Kaggle).
2. Add a link and description to `datasets/README.md`.
3. Note the collection environment (sensor preheat time, temperature, humidity range).

---

## Questions?

Open an [issue](https://github.com/Scarage1/E-Nose/issues) or start a [Discussion](https://github.com/Scarage1/E-Nose/discussions).
