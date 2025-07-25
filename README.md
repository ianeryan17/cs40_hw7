# 🧠 CS40 Homework 7 – λ Calculus & Operational Semantics

This repository contains my solution to **Homework 7** for CS40 (Programming Languages), a course I completed at Tufts University. The assignment explores the evaluation of expressions in a functional language using operational semantics and environment models.

---

## 📚 Topics Covered

- Lambda calculus and closures
- Lexical scoping
- `val`, `lambda`, and `if` evaluation
- Environment structures and closures
- Implementing evaluation rules in μScheme

---

## 📂 Repository Contents

- `eval.ml`: OCaml file implementing the core evaluator logic
- `eval.mli`: Interface file for the evaluator
- `Makefile`: Build configuration
- `tests.ml`: Test cases and evaluation behavior
- `README.md`: You're here!
- `.ocamlinit`, `.gitignore`: Setup files

---

## ▶️ How to Run

1. **Install OCaml** and `make` (we used OCaml 4.x in class):
   ```bash
   brew install ocaml
    ```

2. **Clone this repository:**
   ```bash
   git clone https://github.com/ianeryan17/hw7.git
   cd hw7
   ```

3. **Build and run tests:**
  ```bash
  make test
  ```
