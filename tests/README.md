# Project checks

These tests verify consistency between the Arduino firmware, the Wokwi
circuit, and the dependency list. They do not emulate the Uno or replace a
Wokwi run.

Run from the repository root:

```powershell
python -m unittest discover -s tests -v
```

The suite checks:

- valid JSON and unique part identifiers;
- the expected 31 parts and 113 connections;
- required part types and critical signal wiring;
- the firmware pin constants against that wiring;
- the dependency list;
- non-blocking loop and E-STOP structure.
