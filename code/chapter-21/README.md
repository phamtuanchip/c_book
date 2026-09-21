Chapter 21 code samples - testing & CI

Files:
- test_example.c: simple assert-based tests

CI:
- .github/workflows/ci.yml builds and runs test_example on push/PR

Notes:
- For more advanced testing use Unity or Criterion frameworks.
- Add test runner and report on CI for coverage and artifacts.
